#include "breakpointpanel.h"
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include "debuggerframe.h"
#include "breakpoint.h"
#include "../mupen64plusplus/MupenAPI.h"

enum
{
    break_add = 2,
    break_delete,
    break_disable,
    break_enable,
    break_edit,
    break_last,
};


class BreakpointDialog : public wxDialog
{
    public:
        BreakpointDialog(wxWindow *parent, int id, const wxString &title) : wxDialog(parent, id, title)
        {
            wxPanel *mainpanel = new wxPanel(this, -1);
            wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
            wxPanel *valuepanel = new wxPanel(mainpanel, -1), *typepanel = new wxPanel(mainpanel, -1), *buttonpanel = new wxPanel(mainpanel, -1);;
            mainsizer->Add(valuepanel, 0, wxALL, 4);
            mainsizer->Add(typepanel, 0, wxALL, 4);
            mainsizer->Add(buttonpanel, 0, wxALL, 4);

            wxFlexGridSizer *gridsizer = new wxFlexGridSizer(2, 2, 5);
            gridsizer->AddGrowableCol(1, 1);
            wxStaticText *name_stext = new wxStaticText(valuepanel, -1, _("Name")), *addr_stext = new wxStaticText(valuepanel, -1, _("Address"));
            wxStaticText *length_stext = new wxStaticText(valuepanel, -1, _("Length"));
            name_textctrl = new wxTextCtrl(valuepanel, -1);
            addr_textctrl = new wxTextCtrl(valuepanel, -1);
            length_textctrl = new wxTextCtrl(valuepanel, -1);
            gridsizer->Add(name_stext);
            gridsizer->Add(name_textctrl);
            gridsizer->Add(addr_stext);
            gridsizer->Add(addr_textctrl);
            gridsizer->Add(length_stext);
            gridsizer->Add(length_textctrl);

            exec_cbox = new wxCheckBox(typepanel, -1, _("Execute"));
            read_cbox = new wxCheckBox(typepanel, -1, _("Read"));
            write_cbox = new wxCheckBox(typepanel, -1, _("Write"));
            wxBoxSizer *typesizer = new wxBoxSizer(wxHORIZONTAL);
            typesizer->Add(exec_cbox);
            typesizer->Add(read_cbox);
            typesizer->Add(write_cbox);

            wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
            wxButton *ok_btn = new wxButton(buttonpanel, wxID_OK, _("Ok")), *cancel_btn = new wxButton(buttonpanel, wxID_CANCEL, _("Cancel"));
            buttonsizer->Add(ok_btn, 0, wxRIGHT, 5);
            buttonsizer->Add(cancel_btn);
            ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BreakpointDialog::Ok, this);

            mainpanel->SetSizer(mainsizer);
            valuepanel->SetSizer(gridsizer);
            typepanel->SetSizer(typesizer);
            buttonpanel->SetSizer(buttonsizer);
            SetSize(mainsizer->ComputeFittingWindowSize(this));
            SetReturnCode(wxCANCEL);
            Clear();
        }
        void Clear()
        {
            name_textctrl->Clear();
            addr_textctrl->Clear();
            length_textctrl->SetValue("4");
            exec_cbox->SetValue(false);
            read_cbox->SetValue(false);
            write_cbox->SetValue(false);
        }
        void Ok(wxCommandEvent &evt)
        {
            EndModal(wxOK);
        }
        void UpdateBreakpoint(Breakpoint *bpt)
        {
            wxString name = name_textctrl->GetValue();
            uint32_t address = strtoul(addr_textctrl->GetValue(), 0, 16);
            int length = strtoul(length_textctrl->GetValue(), 0, 16);
            char type = 0;
            if (exec_cbox->GetValue())
                type |= BREAK_TYPE_EXECUTE;
            if (read_cbox->GetValue())
                type |= BREAK_TYPE_READ;
            if (write_cbox->GetValue())
                type |= BREAK_TYPE_WRITE;
            bpt->Update(name, address, length, type);
        }
        void SetValues(Breakpoint *bpt)
        {
            Clear();
            name_textctrl->SetValue(bpt->GetName());
            addr_textctrl->SetValue(wxString::Format("%08X", bpt->GetAddress()));
            length_textctrl->SetValue(wxString::Format("%X", bpt->GetLength()));
            char type = bpt->GetType();
            if (type & BREAK_TYPE_EXECUTE)
                exec_cbox->SetValue(true);
            if (type & BREAK_TYPE_READ)
                read_cbox->SetValue(true);
            if (type & BREAK_TYPE_WRITE)
                write_cbox->SetValue(true);
        }
        BreakValidity IsValid()
        {
            Breakpoint tmp_bpt;
            UpdateBreakpoint(&tmp_bpt);
            return tmp_bpt.IsValid();
        }
        ~BreakpointDialog()
        {
        }
    private:
        wxTextCtrl *name_textctrl;
        wxTextCtrl *addr_textctrl;
        wxTextCtrl *length_textctrl;
        wxCheckBox *exec_cbox;
        wxCheckBox *read_cbox;
        wxCheckBox *write_cbox;
};

BreakpointPanel::BreakpointPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    list = new wxListCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VRULES);
    sizer->Add(list, 1, wxEXPAND);
    SetSizer(sizer);

    list->InsertColumn(0, _("Name"));
    list->InsertColumn(1, _("Address"));
    list->InsertColumn(2, _("Value"));
    list->InsertColumn(3, _("Status"));

    list->Bind(wxEVT_RIGHT_UP, &BreakpointPanel::RClickMenu, this);
    list->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &BreakpointPanel::RClickItem, this);
}

BreakpointPanel::~BreakpointPanel()
{

}

void BreakpointPanel::Update(bool vi)
{
    if (Breakpoint::IsChanged())
    {
        const std::unordered_set<Breakpoint *> *breakpoints = Breakpoint::GetAllBreakpoints();
        if (!breakpoints)
            return;
        list->DeleteAllItems();
        int i = 0;
        for (auto it = breakpoints->begin(); it != breakpoints->end(); it++, i++)
        {
            Breakpoint *bpt = *it;
            wxListItem item;
            item.SetId(i);
            item.SetText(bpt->GetName());
            item.SetData(bpt);
            long id = list->InsertItem(item);

            list->SetItem(id, 0, bpt->GetName());
            list->SetItem(id, 1, wxString::Format("%08X:%X", bpt->GetAddress(), bpt->GetLength()));
            if (bpt->IsEnabled())
                list->SetItem(id, 3, _("Enabled"));
            else
                list->SetItem(id, 3, _("Disabled"));
        }
    }
    for (int i = 0; i < list->GetItemCount(); i++)
    {
        const Breakpoint *bpt = (const Breakpoint *)list->GetItemData(i);
        list->SetItem(i, 2, wxString::Format("%08X", MemRead32(bpt->GetAddress())));
    }
}

void BreakpointPanel::RClickEvent(wxCommandEvent &evt)
{
    int id = evt.GetId();
    switch(id)
    {
        case break_add:
        {
            BreakpointDialog dlg(this, -1, _("Add breakpoint"));
            if (dlg.ShowModal() == wxOK)
            {
                Breakpoint *bpt = new Breakpoint;
                dlg.UpdateBreakpoint(bpt);
                BreakValidity valid = bpt->IsValid();
                if (valid != BREAK_VALID)
                {
                    delete bpt;
                    switch (valid)
                    {
                        case BREAK_INVALID_ADDRESS:
                            wxMessageBox(_("Invalid address"), _("Error"));
                        break;
                        case BREAK_INVALID_TYPE:
                            wxMessageBox(_("No type specified"), _("Error"));
                        break;
                        case BREAK_INVALID_NAME:
                            wxMessageBox(_("No name specified"), _("Error"));
                        break;
                        case BREAK_INVALID_LENGTH:
                            wxMessageBox(_("Invalid length (?)"), _("Error"));
                        break;
                        default:
                        break;
                    }
                }
                else
                {
                    if (!bpt->Add())
                        delete bpt;
                    else
                    {
                        Breakpoint::SetChanged(true);
                        parent->RefreshPanels();
                    }
                }
            }
        }
        break;
        case break_edit:
        {
            BreakpointDialog dlg(this, -1, _("Edit breakpoint"));
            Breakpoint *bpt = (Breakpoint *)list->GetItemData(list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
            dlg.SetValues(bpt);
            if (dlg.ShowModal() == wxOK)
            {
                BreakValidity valid = dlg.IsValid();
                if (valid != BREAK_VALID)
                {
                    switch (valid)
                    {
                        case BREAK_INVALID_ADDRESS:
                            wxMessageBox(_("Invalid address"), _("Error"));
                        break;
                        case BREAK_INVALID_TYPE:
                            wxMessageBox(_("No type specified"), _("Error"));
                        break;
                        case BREAK_INVALID_NAME:
                            wxMessageBox(_("No name specified"), _("Error"));
                        break;
                        case BREAK_INVALID_LENGTH:
                            wxMessageBox(_("Invalid length (?)"), _("Error"));
                        break;
                        default:
                        break;
                    }
                }
                else
                {
                    dlg.UpdateBreakpoint(bpt);
                    Breakpoint::SetChanged(true);
                    parent->RefreshPanels();
                }
            }
        }
        break;
        case break_disable:
        case break_enable:
        case break_delete:
        {
            int item = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            while (item != -1)
            {
                Breakpoint *bpt = (Breakpoint *)list->GetItemData(item);
                if (id == break_disable)
                    bpt->Disable();
                else if (id == break_enable)
                    bpt->Enable();
                else
                    delete bpt;
                item = list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            Breakpoint::SetChanged(true);
            parent->RefreshPanels();
        }
        break;
    }
}

void BreakpointPanel::RClickItem(wxListEvent &evt)
{
    wxMenu menu;
    wxMenuItem *add, *edit, *remove, *disable, *enable;
    add = new wxMenuItem(&menu, break_add, _("New.."));
    menu.Append(add);
    if (list->GetSelectedItemCount() == 1)
    {
        edit = new wxMenuItem(&menu, break_edit, _("Edit.."));
        menu.Append(edit);
    }
    disable = new wxMenuItem(&menu, break_disable, _("Disable"));
    menu.Append(disable);
    enable = new wxMenuItem(&menu, break_enable, _("Enable"));
    menu.Append(enable);
    remove = new wxMenuItem(&menu, break_delete, _("Delete"));
    menu.Append(remove);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &BreakpointPanel::RClickEvent, this, break_add, break_last - 1);

    PopupMenu(&menu);
}

void BreakpointPanel::RClickMenu(wxMouseEvent &evt)
{
    wxMenu menu;
    wxMenuItem *add;
    add = new wxMenuItem(&menu, break_add, _("New.."));
    menu.Append(add);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &BreakpointPanel::RClickEvent, this, break_add, break_last - 1);

    PopupMenu(&menu);
}
