#ifndef BREAKPOINTPANEL_H
#define BREAKPOINTPANEL_H

#include "debugpanel.h"
class wxListCtrl;
class DebuggerFrame;
class wxListEvent;

class BreakpointPanel : public DebugPanel
{
    public:
        BreakpointPanel(DebuggerFrame *parent, int id = -1);
        virtual ~BreakpointPanel();

        void Update(bool vi);
        void RClickMenu(wxMouseEvent &evt);
        void RClickItem(wxListEvent &evt);
        void RClickEvent(wxCommandEvent &evt);

    private:
        void CreateList();
        void UpdateValues();
        bool HasValueChanged(int item, const uint64_t &new_value);
        void GenerateFilterMenu(wxMenu *menu);
        int filter;
        int current_item;
        wxListCtrl *list;
};

#endif // BREAKPOINTPANEL_H
