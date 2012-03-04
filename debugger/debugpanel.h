#ifndef DEBUGPANEL_H
#define DEBUGPANEL_H

#include <wx/panel.h>

class DebuggerFrame;

class DebugPanel : public wxPanel
{
    public:
        DebugPanel(DebuggerFrame *parent, int id = -1);
        virtual ~DebugPanel();

        virtual void Update() {}
        void Rename(wxCommandEvent &evt);

    protected:
        DebuggerFrame *parent;
};

#endif // DEBUGPANEL_H

