#ifndef BREAKPOINTPANEL_H
#define BREAKPOINTPANEL_H

#include "debugpanel.h"
class wxDataViewCtrl;
class wxDataViewColumn;
class BreakDataModel;
class DebuggerFrame;
class wxDataViewEvent;
class Breakpoint;


class BreakpointPanel : public DebugPanel
{
    public:
        BreakpointPanel(DebuggerFrame *parent, int id = -1);
        virtual ~BreakpointPanel();

        void Update(bool vi);
        void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause reason, bool last_update);

        void RClickMenu(wxCommandEvent &evt);
        void RClickItem(wxDataViewEvent &evt);
        void RClickEvent(wxCommandEvent &evt);
        void Deselect(wxMouseEvent &evt);

        void AddBreakpoint(Breakpoint *bpt);

    private:
        void CreateList();
        void UpdateValues();
        bool FilterTest(Breakpoint *bpt);
        bool HasValueChanged(int item, const uint64_t &new_value);
        void GenerateFilterMenu(wxMenu *menu);
        int filter;
        int current_item;

        wxDataViewCtrl *list;
        wxDataViewColumn *name_column;
        wxDataViewColumn *address_column;
        wxDataViewColumn *value_column;
        wxDataViewColumn *enabled_column;
        // If someday there will be a way tofilter what values wxDataViewCtrl actually shows,
        // this could be shared between all instances of BreakpointPanel
        BreakDataModel *data_model;
};

#endif // BREAKPOINTPANEL_H
