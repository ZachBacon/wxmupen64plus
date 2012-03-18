#ifndef DEBUGPANEL_H
#define DEBUGPANEL_H

#include <wx/panel.h>

class DebuggerFrame;
class DebugConsole;

class DebugPanel : public wxPanel
{
    public:
        DebugPanel(DebuggerFrame *parent, int id = -1);
        virtual ~DebugPanel();

        virtual void Update(bool vi) {}
        void Print(wxString &msg);

    protected:
        DebuggerFrame *parent;
        DebugConsole *output;
};

#endif // DEBUGPANEL_H

