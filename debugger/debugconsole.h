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

#ifndef DCONSOLE_H
#define DCONSOLE_H

#include "debugpanel.h"

class DebuggerFrame;
class wxTextCtrl;
class CommandMap;

class DebugConsole : public DebugPanel
{
    public:
        DebugConsole(DebuggerFrame *parent, int id, int type);
        ~DebugConsole();
        void Print(const wxString &msg);
        void Command(wxCommandEvent &evt);

        static void InitCommands();

        void CmdTextDebug(wxString &cmd);

        void CmdPlay(wxString &cmd);
        void CmdPause(wxString &cmd);
        void CmdStep(wxString &cmd);
        void CmdBreak(wxString &cmd);
        void CmdHelp(wxString &cmd);
        void CmdViBreak(wxString &cmd);
        void CmdCls(wxString &cmd);
        void CmdNi(wxString &cmd);

        void RClickMenu(wxMouseEvent &evt);
        void RClickEvent(wxCommandEvent &evt);

    private:
        static CommandMap commands;
        wxTextCtrl *in;
        wxTextCtrl *out;

        int lines;
};

#endif // DCONSOLE_H
