#include "debugconsole.h"
#include "debuggerframe.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/hashmap.h>

typedef void(DebugConsole::*CmdFunc)(wxString &);
WX_DECLARE_STRING_HASH_MAP(CmdFunc, CommandMap);
CommandMap *DebugConsole::commands = 0;

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

    Bind(wxEVT_COMMAND_TEXT_ENTER, &DebugConsole::Command, this);

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

void DebugConsole::CmdHelp(wxString &cmd)
{
    Print("break\tManipulates breakpoints");
    Print("help\tDisplays this help");
    Print("play\tContinues execution");
    Print("pause\tPauses execution");
    Print("step\tExecutes one instruction");
    Print("vibreak\tBreaks at the next vertical interrupt");
    Print("Type \"help <command>\" for detailed instructions.");
}

void DebugConsole::Command(wxCommandEvent &evt)
{
    wxString cmd = in->GetValue();
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
