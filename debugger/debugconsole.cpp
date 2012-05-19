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

#include "debugconsole.h"
#include "debuggerframe.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/hashmap.h>
#include <wx/menu.h>

#include "colors.h"
#include "breakpoint.h"
#include <wx/dcmemory.h>

#include "../mupen64plusplus/MupenAPI.h"
#include <m64p_debugger.h>

extern ptr_DebugGetState DebugGetState;

typedef void(DebugConsole::*CmdFunc)(wxString &);
struct Cmd
{
    CmdFunc func;
    const char *help;
};
WX_DECLARE_STRING_HASH_MAP(Cmd, CommandMap);
CommandMap DebugConsole::commands;

Cmd playcmd = { &DebugConsole::CmdPlay, "\"play|run\" Continues the execution" };
Cmd pausecmd = { &DebugConsole::CmdPause, "\"pause|stop\" Pauses the execution" };
Cmd stepcmd = { &DebugConsole::CmdStep, "\"step\" Executes a single instruction" };
Cmd breakcmd = { &DebugConsole::CmdBreak, "\"b|break|breakpoint <opt>\" Manipulates breakpoints. Opt can be:\n\
    \"a|add <type> <address> [length] [name]\" Adds a new breakpoint\n\
    \"d|del|delete <name|address>\" Deletes breakpoint\n    \"dis|disable <name|address>\" Disables breakpoint\n\
    \"ena|enable <name|address>\" Enables previously disabled breakpoint\n    \"l|list\" Lists all breakpoints\n\
If there are multiple breakpoints with same name, all are affected\n\
Type can be w, r, or x (write, read, execute) or any combination of above." };
Cmd helpcmd = { &DebugConsole::CmdHelp, "\"h|help|?\" Helps poor people" };
Cmd vibreakcmd = { &DebugConsole::CmdViBreak, "\"vi|vibreak\" Breaks at the next vertical interrupt." };
Cmd clearcmd = { &DebugConsole::CmdCls, "\"cls|clear\" Clears this output screen" };
Cmd textcmd = { &DebugConsole::CmdTextDebug, "\"text\" Displays informative information" };
Cmd nicmd = { &DebugConsole::CmdNi, "\"ni\" Shows when next interrupt will happen" };

enum
{
    menu_setmain = 2,
    menu_clear
};

void DebugConsole::InitCommands()
{
    commands["play"] = playcmd;
    commands["run"] = playcmd;
    commands["pause"] = pausecmd;
    commands["stop"] = pausecmd;
    commands["step"] = stepcmd;
    commands["breakpoint"] = breakcmd;
    commands["break"] = breakcmd;
    commands["b"] = breakcmd;
    commands["help"] = helpcmd;
    commands["h"] = helpcmd;
    commands["?"] = helpcmd;
    commands["ohmygodhelpmeplease"] = helpcmd;
    commands["vibreak"] = vibreakcmd;
    commands["vi"] = vibreakcmd;
    commands["cls"] = clearcmd;
    commands["clear"] = clearcmd;
    commands["text"] = textcmd;
    commands["ni"] = nicmd;
}

DebugConsole::DebugConsole(DebuggerFrame *parent_, int id, int type) : DebugPanel(parent_, id, type)
{
    if (commands.empty())
        InitCommands();

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    in = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    out = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
    sizer->Add(out, 1, wxEXPAND);
    sizer->Add(in, 0, wxEXPAND);

    in->Bind(wxEVT_COMMAND_TEXT_ENTER, &DebugConsole::Command, this);
    out->Bind(wxEVT_RIGHT_UP, &DebugConsole::RClickMenu, this);

    lines = 0;
}

DebugConsole::~DebugConsole()
{
}

void DebugConsole::CmdTextDebug(wxString &cmd)
{
    // You have to use dc to get this information, right?
    wxMemoryDC dc;
    dc.SetFont(*g_main_font);

    const char *arg = cmd.c_str();
    while (arg[0] != 0 && arg[0] != ' ')
        arg++;
    while (arg[0] != 0 && arg[0] == ' ')
        arg++;

    if (!arg[0])
    {
        wxFontMetrics metr = dc.GetFontMetrics();
        Print(wxString::Format("W: %d, H: %d (W: %d, H: %d) // A: %d D: %d // IL: %d, EL: %d", dc.GetCharWidth(), dc.GetCharHeight(),
                               metr.averageWidth, metr.height, metr.ascent, metr.descent, metr.internalLeading, metr.externalLeading));
    }
    else
    {
        wxSize extend = dc.GetTextExtent(arg);
        Print(wxString::Format("W: %d, H: %d", extend.x, extend.y));
    }
}

void DebugConsole::CmdPlay(wxString &cmd)
{
    parent->Run();
    Print("Playing");
}

void DebugConsole::CmdPause(wxString &cmd)
{
    parent->Pause();
}

void DebugConsole::CmdStep(wxString &cmd)
{
    parent->Step();
}

void DebugConsole::CmdBreak(wxString &cmd)
{
    const char *cmd_cstr = cmd.c_str();
    strtok((char *)cmd_cstr, " ");
    char *arg = strtok(0, " ");
    if (!arg)
    {
        Print(breakcmd.help);
        return;
    }
    if (arg[0] == 'a' && (arg[1] == 0 || strcmp(arg, "add") == 0))
    {
        const char *type_str = strtok(0, " "), *address_str = strtok(0, " "), *length_str = strtok(0, " "), *name = strtok(0, "");
        if (!address_str)
        {
            Print(breakcmd.help);
            return;
        }
        uint32_t address = strtoul(address_str, 0, 16);
        uint32_t length = 0;
        if (length_str)
            length = strtoul(length_str, 0, 16);
        if (!length)
            length = 4;

        char type = 0;
        while (type_str[0])
        {
            if (type_str[0] == 'x')
                type |= BREAK_TYPE_EXECUTE;
            else if (type_str[0] == 'r')
                type |= BREAK_TYPE_READ;
            else if (type_str[0] == 'w')
                type |= BREAK_TYPE_WRITE;
            type_str++;
        }
        if (!type)
        {
            Print("Invalid type.");
            return;
        }


        if (!name)
            name = "unnamed";

        Breakpoint *bpt = new Breakpoint(name, address, length, type);
        if (!parent->AddBreakpoint(bpt))
        {
            if (bpt->IsValid() == BREAK_INVALID_ADDRESS)
                Print("Invalid address.");
            else if (Breakpoint *other = parent->FindBreakpoint(address, length))
                Print(wxString::Format("There is already breakpoint named \"%s\" at %08X.", other->GetName(), other->GetAddress()));
            else
                Print("Maximum number of breakpoints reached.");
            delete bpt;
        }
    }
    else if (arg[0] == 'd')
    {
        const char *name = strtok(0, "");
        if (!name)
        {
            Print(breakcmd.help);
            return;
        }
        int break_amt;
        auto breaks = parent->FindBreakpointsByName(name, &break_amt);
        if (arg[1] == 0 || strcmp(arg, "del") == 0 || strcmp(arg, "delete") == 0)
        {
            for (int i = 0; i < break_amt; i++)
            {
                parent->DeleteBreakpoint(breaks[i], i == break_amt - 1);
            }
        }
        else if (strcmp(arg, "dis") == 0 || strcmp(arg, "disable") == 0)
        {
            for (int i = 0; i < break_amt; i++)
            {
                parent->EnableBreakpoint(breaks[i], false, i == break_amt - 1);
            }
        }
    }
    else if (arg[0] == 'e' && (strcmp(arg, "ena") == 0 || strcmp(arg, "enable") == 0))
    {
        const char *name = strtok(0, "");
        if (!name)
        {
            Print(breakcmd.help);
            return;
        }
        int break_amt;
        auto breaks = parent->FindBreakpointsByName(name, &break_amt);
        for (int i = 0; i < break_amt; i++)
        {
            parent->EnableBreakpoint(breaks[i], true, i == break_amt - 1);
        }
    }
    else if (arg[0] == 'l' && (arg[1] == 0 || strcmp(arg, "list") == 0))
    {
        Print("Name\t\tType\tAddress:length\tenabled");
        const BreakContainer *breaks = parent->GetBreakpoints();
        int count = breaks->size();
        auto it = breaks->begin();
        for (int i = 0; i < count; i++, ++it)
        {
            Breakpoint *bpt = it->second;
            char type_str[4], *type_pos = type_str, type = bpt->GetType();
            if (type & BREAK_TYPE_READ)
                *(type_pos++) = 'r';
            if (type & BREAK_TYPE_WRITE)
                *(type_pos++) = 'w';
            if (type & BREAK_TYPE_EXECUTE)
                *(type_pos++) = 'x';
            *type_pos = 0;
            char enabled;
            if (bpt->IsEnabled())
                enabled = 'y';
            else
                enabled = 'n';
            wxString name = bpt->GetName();
            if (name.length() < 10) // Just guessing, and tab width might be really different on different platforms
                name.Append('\t');
            Print(wxString::Format("%s\t%s\t%08X:%X\t%c", name, type_str, bpt->GetAddress(), bpt->GetLength(), enabled));
        }
    }
    else
        Print(breakcmd.help);
}

void DebugConsole::CmdViBreak(wxString &cmd)
{
    parent->ViBreak();
}

void DebugConsole::CmdNi(wxString &cmd)
{
    Print(wxString::Format("Next interrupt happens when count >= %X", (*DebugGetState)(M64P_DBG_CPU_NEXT_INTERRUPT)));
}

void DebugConsole::CmdCls(wxString &cmd)
{
    out->Clear();
    lines = 0;
}

void DebugConsole::CmdHelp(wxString &cmd)
{
    const char *cmd_cstr = cmd.c_str();
    strtok((char *)cmd_cstr, " ");
    char *arg = strtok(0, " ");
    if (!arg)
    {
        Print("break\tManipulates breakpoints");
        Print("help\tDisplays this help");
        Print("play\tContinues execution");
        Print("pause\tPauses execution");
        Print("step\tExecutes one instruction");
        Print("clear\tClears the screen");
        Print("ni\tTells when next interrupt happens");
        Print("Type \"help <command>\" for detailed instructions.");
    }
    else
    {
        CommandMap::iterator it = commands.find(arg);
        if(it != commands.end())
            Print(it->second.help);
        else
            Print(wxString::Format("Unknown command \"%s\". Type \"help\" for list of commands.", cmd));
    }
}

void DebugConsole::Command(wxCommandEvent &evt)
{
    wxString cmd = in->GetValue();
    if(cmd.empty())
        return;

    cmd.MakeLower(); // Maybe shouldn't?
    wxString cmd_name = cmd.substr(0, cmd.find_first_of(' '));
    CommandMap::iterator it = commands.find(cmd_name);
    if(it != commands.end())
    {
        CmdFunc func = it->second.func;
        ((this)->*(func))(cmd);
    }
    else
    {
        Print(wxString::Format("Unknown command \"%s\". Type \"help\" for list of commands.", cmd));
    }
    in->Clear();
}

void DebugConsole::Print(const wxString &msg)
{
    if (lines)
        out->AppendText("\n");
    if (lines > 100) // Too small?
        out->Remove(0, out->GetLineLength(0) + 1);
    else
        lines++;

    out->AppendText(msg);
}

void DebugConsole::RClickEvent(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case menu_clear:
            out->Clear();
            lines = 0;
        break;
        case menu_setmain:
            parent->SetMainOutput(this);
        break;
        default:
        break;
    }
}

void DebugConsole::RClickMenu(wxMouseEvent &evt)
{
    wxMenu menu;
    wxMenuItem *setmain, *clear;
    if (parent->GetMainOutput() != this)
    {
        setmain = new wxMenuItem(&menu, menu_setmain, _("Set to main output")); // is "to" correct preposition here? D:
        menu.Append(setmain);
    }

    clear = new wxMenuItem(&menu, menu_clear, _("Clear"));
    menu.Append(clear);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &DebugConsole::RClickEvent, this);

    PopupMenu(&menu);
}

