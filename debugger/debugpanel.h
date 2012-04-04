#ifndef DEBUGPANEL_H
#define DEBUGPANEL_H

#include <wx/panel.h>

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
        DebugPanel(DebuggerFrame *parent, int id = -1);
        virtual ~DebugPanel();

        virtual void Update(bool vi) {}
        virtual void Reset() {}
        void Print(const wxString &msg);

        DebuggerFrame *GetParent() { return parent; }


        virtual void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update) {}

    protected:
        DebuggerFrame *parent;
        DebugConsole *output;
};

#endif // DEBUGPANEL_H

