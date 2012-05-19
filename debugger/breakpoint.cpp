/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus debugger                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Markus Heikkinen                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "breakpoint.h"

#define uint32 uint32_t
#include <../debugger/dbg_breakpoints.h> // sigh
#include <m64p_debugger.h>
#include "../mupen64plusplus/MupenAPI.h"

using namespace std;

extern ptr_DebugBreakpointCommand DebugBreakpointCommand;
extern ptr_DebugBreakpointLookup DebugBreakpointLookup;

template class unique_ptr<Breakpoint *[]>;
template class unordered_map<uint32_t, Breakpoint *>;
template class BreakContainer;

bool CmpStringPointers::operator()(const wxString *a, const wxString *b) const
{
    return *a == *b;
}

size_t HashStringPointer::operator()(const wxString *val) const
{
    const char *str = val->c_str();
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

BreakpointInterface::BreakpointInterface()
{

}

BreakpointInterface::~BreakpointInterface()
{
    Clear();
}

bool BreakpointInterface::AddToCore(Breakpoint *bpt)
{
    breakpoint raw_bpt;
    raw_bpt.address = bpt->address;
    raw_bpt.endaddr = bpt->address + bpt->length - 1;
    raw_bpt.flags = BPT_FLAG_ENABLED | (bpt->type & BREAK_TYPE_ALL);

    bpt->id = (*DebugBreakpointCommand)(M64P_BKP_CMD_ADD_STRUCT, 0, &raw_bpt);
    if (bpt->id == -1)
        return false;

    return true;
}

bool BreakpointInterface::Add(Breakpoint *bpt)
{
    if (bpt->IsValid() != BREAK_VALID)
        return false;

    if (Find(bpt->address, bpt->length))
        return false; // Well, this could edit the existing breakpoint to merge or something it with this one

    if (!AddToCore(bpt))
        return false;

    for (int i = 0; i < bpt->length; i++)
        breakmap.insert(pair<uint32_t, Breakpoint *>(bpt->address + i, bpt));
    breaks.insert(pair<wxString *, Breakpoint *>(&bpt->name, bpt));
    return true;
}

bool BreakpointInterface::Update(Breakpoint *bpt, const wxString &name, uint32_t address, int length, char type)
{
    if (Breakpoint::IsValid(name, address, length, type) != BREAK_VALID)
        return false;

    if (name != bpt->name)
    {
        for (auto range = breaks.equal_range(&bpt->name); range.first != range.second; ++range.first)
        {
            if (range.first->second == bpt)
            {
                breaks.erase(range.first);
                break;
            }
        }
        bpt->name = name;
        breaks.insert(pair<wxString *, Breakpoint *>(&bpt->name, bpt));
    }

    if (!bpt->IsEnabled())
    {
        bpt->SetValues(address, length, type);
        return true;
    }
    if (address != bpt->address || length != bpt->length)
    {
        Breakpoint *other = Find(address, length);
        if (other && other != bpt)
            return false;

        for (int i = 0; i < bpt->length; i++)
            breakmap.erase(bpt->address + i);
        for (int i = 0; i < length; i++)
            breakmap.insert(pair<uint32_t, Breakpoint *>(address + i, bpt));
    }
    bpt->SetValues(address, length, type);
    breakpoint raw_bpt;
    raw_bpt.address = address;
    raw_bpt.endaddr = address + length - 1;
    raw_bpt.flags = (type & BREAK_TYPE_ALL) | BPT_FLAG_ENABLED;

    (*DebugBreakpointCommand)(M64P_BKP_CMD_REPLACE, bpt->id, &raw_bpt);
    return true;

}

void BreakpointInterface::RemoveFromCore(Breakpoint *bpt)
{
    if (bpt->id == -1)
        return;

    int old_id = bpt->id;
    (*DebugBreakpointCommand)(M64P_BKP_CMD_REMOVE_IDX, bpt->id, 0);
    bpt->id = -1;

    for (auto it = breaks.begin(); it != breaks.end(); ++it)
    {
        Breakpoint *entry = it->second;
         if (entry->id > old_id) // Core does this too, so it has to be mimiced (maybe shouldn't use ids at all)
        {
            entry->id -= 1;
        }
    }
}

unique_ptr<Breakpoint *[]> BreakpointInterface::FindByName(const wxString &name, int *amt)
{
    int count = breaks.count(&name);
    *amt = count;
    if (!count)
        return 0;

    unique_ptr<Breakpoint *[]> ret(new Breakpoint*[count]);
    auto range = breaks.equal_range(&name);
    auto it = range.first;
    for (int i = 0; i < count; i++, ++it)
    {
        ret[i] = it->second;
    }
    return ret;
}

void BreakpointInterface::Delete(Breakpoint *bpt)
{
    RemoveFromCore(bpt);
    for (int i = 0; i < bpt->length; i++)
        breakmap.erase(bpt->address + i);

    auto range = breaks.equal_range(&bpt->GetName());
    for (auto it = range.first; it != range.second; ++it)
    {
        if (it->second == bpt)
        {
            breaks.erase(it);
            break;
        }
    }
    delete bpt;
}

bool BreakpointInterface::Enable(Breakpoint *bpt)
{
    if (bpt->IsEnabled())
        return true;

    return AddToCore(bpt);
}

void BreakpointInterface::Disable(Breakpoint *bpt)
{
    if (!bpt->IsEnabled())
        return;

    RemoveFromCore(bpt);
}

Breakpoint *BreakpointInterface::Find(uint32_t address, uint32_t length)
{
    std::unordered_map<uint32_t, Breakpoint *>::iterator it, end = breakmap.end();
    for (uint32_t i = 0; i < length; i++)
    {
        it = breakmap.find(address + i);
        if (it != end)
            return it->second;
    }
    return 0;
}

void BreakpointInterface::Clear()
{
    for (auto it = breaks.begin(); it != breaks.end(); ++it)
    {
        Breakpoint *bpt = it->second;

        if (bpt->id != -1)
            (*DebugBreakpointCommand)(M64P_BKP_CMD_REMOVE_IDX, bpt->id, 0);

        delete bpt;
    }
    breakmap.clear();
    breaks.clear();
}

// ----------------------------------------------------------------------

Breakpoint::Breakpoint()
{
    name = "";
    address = 0;
    type = 0;
    length = 0;
    id = -1;
}

Breakpoint::Breakpoint(const wxString &name_, uint32_t address_, int length_, char type_)
{
    id = -1;
    SetValues(name_, address_, length_, type_);
}

Breakpoint::~Breakpoint()
{
}

void Breakpoint::SetValues(const wxString &name_, uint32_t address_, int length_, char type_)
{
    name = name_;
    type = type_;
    address = address_;
    length = length_;
}

void Breakpoint::SetValues(uint32_t address_, int length_, char type_)
{
    type = type_;
    address = address_;
    length = length_;
}

BreakValidity Breakpoint::IsValid() const
{
    return IsValid(name, address, length, type);
}

BreakValidity Breakpoint::IsValid(const wxString &name, uint32_t address, int length, char type)
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
