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

#ifndef DEBUGPANEL_H
#define DEBUGPANEL_H

#include <wx/panel.h>

class DebugConfigOut;
class DebugConfigSection;
class DebuggerFrame;
class DebugConsole;
class Breakpoint;

enum BreakUpdateCause
{
    BREAK_REMOVED,
    BREAK_ADDED,
    BREAK_CHANGED
};

class DebugPanel : public wxPanel
{
    public:
        DebugPanel(DebuggerFrame *parent, int id, int type);
        virtual ~DebugPanel();

        virtual void SaveConfig(DebugConfigOut &config, DebugConfigSection &section);
        virtual void Update(bool vi) {}
        virtual void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update) {}

        virtual void Reset() {}
        void Print(const wxString &msg);

        DebuggerFrame *GetParent() { return parent; }
        int GetType() { return type; }

    protected:
        DebuggerFrame *parent;

    private:
        DebugConsole *output;
        int type;
};

#endif // DEBUGPANEL_H

