#include "debugconsole.h"
#include "debuggerframe.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/hashmap.h>
#include <wx/menu.h>

#include "colors.h"
#include <wx/dcmemory.h>

#include "../mupen64plusplus/MupenAPI.h"

typedef void(DebugConsole::*CmdFunc)(wxString &);
struct Cmd
{
    CmdFunc func;
    const char *help;
};
WX_DECLARE_STRING_HASH_MAP(Cmd, CommandMap);
CommandMap *DebugConsole::commands = 0;

Cmd playcmd = { &DebugConsole::CmdPlay, "\"play|run\" Continues the execution" };
Cmd pausecmd = { &DebugConsole::CmdPause, "\"pause|stop\" Pauses the execution" };
Cmd stepcmd = { &DebugConsole::CmdStep, "\"step\" Executes a single instruction" };
Cmd breakcmd = { &DebugConsole::CmdBreak, "\"b|break|breakpoint <opt>\" Manipulates breakpoints. Opt can be:\n    \"a|add <type> <address>\" Adds a new breakpoint\n\
    \"d|del <id|address>\" Deletes breakpoint\n    \"dis|disable <id|address>\" Disables breakpoint\n\
    \"ena|enable <id|address>\" Enables previously disabled breakpoint\n    \"l|list\" Lists all breakpoints\n\
Commands accepting either address or breakpoint id will assume the argument to be id in conflicting situations, unless the argument is prefixed by either \"0\" or \"0x\".\n\
Type can be w, r, or x (write, read, execute) or any combination of above." };
Cmd helpcmd = { &DebugConsole::CmdHelp, "\"h|help|?\" Helps poor people" };
Cmd vibreakcmd = { &DebugConsole::CmdViBreak, "\"vi|vibreak\" Breaks at the next vertical interrupt. Used mainly for debugging the debugger" };
Cmd clearcmd = { &DebugConsole::CmdCls, "\"cls|clear\" Clears this output screen" };
Cmd textcmd = { &DebugConsole::CmdTextDebug, "\"text\" Displays informative information" };

enum
{
    menu_setmain = 2,
    menu_clear
};

void DebugConsole::InitCommands()
{
    commands = new CommandMap;
    (*commands)["play"] = playcmd;
    (*commands)["run"] = playcmd;
    (*commands)["pause"] = pausecmd;
    (*commands)["stop"] = pausecmd;
    (*commands)["step"] = stepcmd;
    (*commands)["breakpoint"] = breakcmd;
    (*commands)["break"] = breakcmd;
    (*commands)["b"] = breakcmd;
    (*commands)["help"] = helpcmd;
    (*commands)["h"] = helpcmd;
    (*commands)["?"] = helpcmd;
    (*commands)["ohmygodhelpmeplease"] = helpcmd;
    (*commands)["vibreak"] = vibreakcmd;
    (*commands)["vi"] = vibreakcmd;
    (*commands)["cls"] = clearcmd;
    (*commands)["clear"] = clearcmd;
    (*commands)["text"] = textcmd;
}

DebugConsole::DebugConsole(DebuggerFrame *parent_, int id) : DebugPanel(parent_, id)
{
    if (!commands)
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
    parent->ConsoleClosed(this);
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
}

void DebugConsole::CmdViBreak(wxString &cmd)
{
    parent->ViBreak();
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
        Print("Type \"help <command>\" for detailed instructions.");
    }
    else
    {
        CommandMap::iterator it = commands->find(arg);
        if(it != commands->end())
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
    CommandMap::iterator it = commands->find(cmd_name);
    if(it != commands->end())
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

