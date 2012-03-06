#include "debugconsole.h"
#include "debuggerframe.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/hashmap.h>
#include <wx/menu.h>

typedef void(DebugConsole::*CmdFunc)(wxString &);
WX_DECLARE_STRING_HASH_MAP(CmdFunc, CommandMap);
CommandMap *DebugConsole::commands = 0;

enum
{
    menu_setmain = 2,
    menu_clear
};

void DebugConsole::InitCommands()
{
    commands = new CommandMap;
    (*commands)["play"] = &DebugConsole::CmdPlay;
    (*commands)["run"] = &DebugConsole::CmdPlay;
    (*commands)["pause"] = &DebugConsole::CmdPause;
    (*commands)["stop"] = &DebugConsole::CmdPause;
    (*commands)["step"] = &DebugConsole::CmdStep;
    (*commands)["breakpoint"] = &DebugConsole::CmdBreak;
    (*commands)["break"] = &DebugConsole::CmdBreak;
    (*commands)["b"] = &DebugConsole::CmdBreak;
    (*commands)["help"] = &DebugConsole::CmdHelp;
    (*commands)["h"] = &DebugConsole::CmdHelp;
    (*commands)["?"] = &DebugConsole::CmdHelp;
    (*commands)["ohmygodhelpmeplease"] = &DebugConsole::CmdHelp;
    (*commands)["vibreak"] = &DebugConsole::CmdViBreak;
    (*commands)["vi"] = &DebugConsole::CmdViBreak;
    (*commands)["cls"] = &DebugConsole::CmdCls;
    (*commands)["clear"] = &DebugConsole::CmdCls;
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
    Print("break\tManipulates breakpoints");
    Print("help\tDisplays this help");
    Print("play\tContinues execution");
    Print("pause\tPauses execution");
    Print("step\tExecutes one instruction");
    Print("cls\tClears the screen");
    Print("Type \"help <command>\" for detailed instructions.");
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
        CmdFunc func = it->second;
        ((this)->*(func))(cmd);
    }
    else
    {
        Print(wxString::Format("Unknown command \"%s\". Type \"help\" for list of commands.", cmd));
    }
    in->Clear();
}

void DebugConsole::Print(wxString msg)
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

