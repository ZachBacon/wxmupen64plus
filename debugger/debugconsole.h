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
        static CommandMap *commands;
        wxTextCtrl *in;
        wxTextCtrl *out;

        int lines;
};

#endif // DCONSOLE_H
