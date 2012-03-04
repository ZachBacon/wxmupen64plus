#ifndef DCONSOLE_H
#define DCONSOLE_H

#include "debugpanel.h"

class DebuggerFrame;
class wxTextCtrl;
class CommandMap;

class DebugConsole : public DebugPanel
{
    public:
        DebugConsole(DebuggerFrame *parent, int id = -1);
        ~DebugConsole();
        void Print(wxString msg);
        void Command(wxCommandEvent &evt);

        static void InitCommands();

        void CmdPlay(wxString &cmd);
        void CmdPause(wxString &cmd);
        void CmdStep(wxString &cmd);
        void CmdBreak(wxString &cmd);
        void CmdHelp(wxString &cmd);

    private:
        static CommandMap *commands;
        wxTextCtrl *in;
        wxTextCtrl *out;

        int lines;
};

#endif // DCONSOLE_H
