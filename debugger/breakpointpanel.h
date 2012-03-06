#ifndef BREAKPOINTPANEL_H
#define BREAKPOINTPANEL_H

#include "debugpanel.h"
class wxListCtrl;
class DebuggerFrame;

class BreakpointPanel : public DebugPanel
{
    public:
        BreakpointPanel(DebuggerFrame *parent, int id = -1);
        virtual ~BreakpointPanel();

        void Update(bool vi);
        void RClickMenu(wxMouseEvent &evt);
        void RClickEvent(wxCommandEvent &evt);
    private:
        int current_item;
        wxListCtrl *list;
};

#endif // BREAKPOINTPANEL_H
