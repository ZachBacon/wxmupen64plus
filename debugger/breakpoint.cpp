#include "breakpoint.h"

#define uint32 uint32_t
#include <../debugger/dbg_breakpoints.h> // sigh
#include <m64p_debugger.h>
#include "../mupen64plusplus/MupenAPI.h"

extern ptr_DebugBreakpointCommand DebugBreakpointCommand;
extern ptr_DebugBreakpointLookup DebugBreakpointLookup;
template class std::unordered_map<uint32_t, Breakpoint *>;
std::unordered_map<uint32_t, Breakpoint *> *Breakpoint::breakmap = 0;
std::unordered_set<Breakpoint *> *Breakpoint::breakset = 0;
bool Breakpoint::changed = true;

Breakpoint::Breakpoint()
{
    if (!breakmap)
    {
        breakmap = new std::unordered_map<uint32_t, Breakpoint *>;
        breakset = new std::unordered_set<Breakpoint *>;
    }
    name = "";
    address = 0;
    type = 0;
    length = 0;
    id = -1;
    active = false;
}

Breakpoint::Breakpoint(const wxString &name_, uint32_t address_, int length_, char type_)
{
    if (!breakmap)
    {
        breakmap = new std::unordered_map<uint32_t, Breakpoint *>;
        breakset = new std::unordered_set<Breakpoint *>;
    }
    id = -1;
    active = false;
    Update(name_, address_, length_, type_);
}

bool Breakpoint::Update(const wxString &name_, uint32_t address_, int length_, char type_)
{
    name = name_;
    type = type_;
    if (!active)
    {
        address = address_;
        length = length_;
        return true;
    }

    if (address_ != address || length_ != length)
    {
        Breakpoint *other = Find(address_);
        if (other && other != this)
            return false;

        for (int i = 0; i < length; i++)
            breakmap->erase(address + i);
        address = address_;
        length = length_;
        for (int i = 0; i < length; i++)
            breakmap->insert(std::pair<uint32_t, Breakpoint *>(address + i, this));
    }
    breakpoint bpt;
    bpt.address = address;
    bpt.endaddr = address + length - 1;
    bpt.flags = type & BREAK_TYPE_ALL;
    if (enabled)
        bpt.flags |= BPT_FLAG_ENABLED;

    (*DebugBreakpointCommand)(M64P_BKP_CMD_REPLACE, id, &bpt);
    return true;

}

Breakpoint::~Breakpoint()
{
    Remove();
}


bool Breakpoint::Add()
{
    if (IsValid() != BREAK_VALID)
        return false;
    if (Find(address))
        return false; // Well, this could edit the existing breakpoint to include types this one has

    breakpoint bpt;
    bpt.address = address;
    bpt.endaddr = address + length - 1;
    bpt.flags = BPT_FLAG_ENABLED | (type & BREAK_TYPE_ALL);

    id = (*DebugBreakpointCommand)(M64P_BKP_CMD_ADD_STRUCT, 0, &bpt);
    if (id == -1)
        return false;

    enabled = true;
    active = true;
    for (int i = 0; i < length; i++)
        breakmap->insert(std::pair<uint32_t, Breakpoint *>(address + i, this));
    breakset->insert(this);
    return true;
}

void Breakpoint::Remove()
{
    if (id == -1)
        return;

    (*DebugBreakpointCommand)(M64P_BKP_CMD_REMOVE_IDX, id, 0);
    id = -1;
    for (int i = 0; i < length; i++)
        breakmap->erase(address + i);
    breakset->erase(this);
    active = false;
}

void Breakpoint::Enable()
{
    if (enabled)
        return;

    (*DebugBreakpointCommand)(M64P_BKP_CMD_ENABLE, id, 0);
    enabled = true;
}

void Breakpoint::Disable()
{
    if (!enabled)
        return;

    (*DebugBreakpointCommand)(M64P_BKP_CMD_DISABLE, id, 0);
    enabled = false;
}

BreakValidity Breakpoint::IsValid() const
{
    if (!(type & BREAK_TYPE_ALL))
        return BREAK_INVALID_TYPE;
    if (!MemIsValid(address))
        return BREAK_INVALID_ADDRESS;
    if (!length)
        return BREAK_INVALID_LENGTH;
    if (name.empty())
        return BREAK_INVALID_NAME;
    return BREAK_VALID;
}

Breakpoint *Breakpoint::Find(uint32_t address)
{
    if (!breakmap)
        return 0;

    auto it = breakmap->find(address);
    if (it == breakmap->end())
        return 0;
    return it->second;
}
