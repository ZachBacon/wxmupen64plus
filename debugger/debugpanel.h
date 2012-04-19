#ifndef DEBUGPANEL_H
#define DEBUGPANEL_H

#include <wx/panel.h>

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

        virtual void SaveConfig(DebugConfigSection &config) {}
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

