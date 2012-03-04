#include "breakpointpanel.h"
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/dialog.h>

enum
{
    break_add = 2,
    break_delete,
    break_disable,
    break_edit,
    break_last
};


class BreakpointDialog : public wxDialog
{
    public:
        BreakpointDialog()
        {
        }
        ~BreakpointDialog()
        {
        }
};

BreakpointPanel::BreakpointPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    list = new wxListCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES);
    sizer->Add(list, 1, wxEXPAND);
    SetSizer(sizer);


    list->InsertColumn(0, "Name");
    list->InsertColumn(1, "Address");
    list->InsertColumn(2, "Status");

    list->Bind(wxEVT_RIGHT_UP, &BreakpointPanel::RClickMenu, this);
}

BreakpointPanel::~BreakpointPanel()
{

}

void BreakpointPanel::Update()
{

}

void BreakpointPanel::RClickEvent(wxCommandEvent &evt)
{
    switch(evt.GetId())
    {
        case break_add:
        {
            //BreakpointDialog *dlg = new
        }
        break;
        case break_edit:
        break;
        case break_disable:

        break;
        case break_delete:
        break;
    }
}

void BreakpointPanel::RClickMenu(wxMouseEvent &evt)
{
    wxMenu menu;
    wxMenuItem *add, *edit, *remove, *disable;
    add = new wxMenuItem(&menu, break_add, _("New.."));
    menu.Append(add);

    int hitflags;
    current_item = list->HitTest(evt.GetPosition(), hitflags);
    if(current_item != wxNOT_FOUND)
    {
        edit = new wxMenuItem(&menu, break_edit, _("Edit.."));
        disable = new wxMenuItem(&menu, break_disable, _("Disable"));
        remove = new wxMenuItem(&menu, break_delete, _("Delete"));

        menu.Append(edit);
        menu.Append(disable);
        menu.Append(remove);
    }

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &BreakpointPanel::RClickEvent, this, break_add, break_last - 1);

    PopupMenu(&menu);
}
