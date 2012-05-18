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

#ifndef BREAKPOINT_H
#define BREAKPOINT_H
#include <stdint.h>
#include <wx/string.h>
#include <unordered_map>
#include <memory>

// These correspond to core's dbg_breakpoint.h (for now)
#define BREAK_TYPE_EXECUTE 0x20
#define BREAK_TYPE_READ 0x8
#define BREAK_TYPE_WRITE 0x10
#define BREAK_TYPE_ALL (BREAK_TYPE_EXECUTE | BREAK_TYPE_READ | BREAK_TYPE_WRITE)

class Breakpoint;
class HashStringPointer
{
    public:
    size_t operator()(const wxString *val) const;
};
class CmpStringPointers
{
    public:
    bool operator()(const wxString *a, const wxString *b) const;
};
#define BreakContainer std::unordered_multimap<const wxString *, Breakpoint *, HashStringPointer, CmpStringPointers>

extern template class std::unique_ptr<Breakpoint *[]>;
extern template class std::unordered_map<uint32_t, Breakpoint *>;
extern template class BreakContainer;

enum BreakValidity
{
    BREAK_VALID = 0,
    BREAK_INVALID_TYPE,
    BREAK_INVALID_ADDRESS,
    BREAK_INVALID_NAME,
    BREAK_INVALID_LENGTH
};

class BreakpointInterface
{
    public:
        BreakpointInterface();
        ~BreakpointInterface();

        bool Add(Breakpoint *bpt);
        bool Update(Breakpoint *bpt, const wxString &name, uint32_t address, int length, char type);
        void Delete(Breakpoint *bpt);
        bool Enable(Breakpoint *bpt);
        void Disable(Breakpoint *bpt);

        void Clear();

        Breakpoint *Find(uint32_t address, uint32_t length = 1);
        std::unique_ptr<Breakpoint *[]> FindByName(const wxString &name, int *amt);
        const BreakContainer *GetAllBreakpoints() { return &breaks; }

    private:
        bool AddToCore(Breakpoint *bpt);
        void RemoveFromCore(Breakpoint *bpt);
        
        // I don't think hash table is the best option for data ranges far aways each other, but it shall be enough for now
        std::unordered_map<uint32_t, Breakpoint *> breakmap;
        BreakContainer breaks;
};

class Breakpoint
{
    friend class BreakpointInterface;
    public:
        Breakpoint();
        Breakpoint(const wxString &name, uint32_t address, int length, char type);
        ~Breakpoint();

        BreakValidity IsValid() const;
        static BreakValidity IsValid(const wxString &name, uint32_t address, int length, char type);

        bool IsEnabled() const { return id != -1; }
        char GetType() const { return type; }
        const wxString &GetName() const { return name; }
        uint32_t GetAddress() const { return address; }
        int GetLength() const { return length; }

    private:
        // Setting values outside of creation should be done through BreakpointInterface
        void SetValues(const wxString &name, uint32_t address, int length, char type);
        void SetValues(uint32_t address_, int length_, char type_);

        wxString name;
        int id;
        uint32_t address;
        int length;
        char type;
};

#endif // BREAKPOINT_H

