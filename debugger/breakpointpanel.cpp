#include "breakpointpanel.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/msgdlg.h>

#include "breakpoint.h"
#include "debuggerframe.h"
#include "dv_treelist.h"
#include "../mupen64plusplus/MupenAPI.h"

#define BREAK_FILTER_DISABLED 0x1
#define BREAK_FILTER_ENABLED 0x2
#define BREAK_FILTER_EXEC 0x4
#define BREAK_FILTER_READ 0x8
#define BREAK_FILTER_WRITE 0x10
#define BREAK_FILTER_ALL 0x1f

enum
{
    break_add = 2,
    break_delete,
    break_disable,
    break_enable,
    break_edit,
    break_f_disabled,
    break_f_enabled,
    break_f_exec,
    break_f_read,
    break_f_write,
    break_last
};

enum
{
    name_col = 0,
    address_col,
    length_col,
    value_col,
    enabled_col
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
            name_textctrl = new wxTextCtrl(valuepanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            addr_textctrl = new wxTextCtrl(valuepanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            length_textctrl = new wxTextCtrl(valuepanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            gridsizer->Add(name_stext);
            gridsizer->Add(name_textctrl);
            gridsizer->Add(addr_stext);
            gridsizer->Add(addr_textctrl);
            gridsizer->Add(length_stext);
            gridsizer->Add(length_textctrl);

            exec_cbox = new wxCheckBox(typepanel, -1, _("Execute"), wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
            read_cbox = new wxCheckBox(typepanel, -1, _("Read"), wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
            write_cbox = new wxCheckBox(typepanel, -1, _("Write"), wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
            wxBoxSizer *typesizer = new wxBoxSizer(wxHORIZONTAL);
            typesizer->Add(exec_cbox);
            typesizer->Add(read_cbox);
            typesizer->Add(write_cbox);

            wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
            wxButton *ok_btn = new wxButton(buttonpanel, wxID_OK, _("Ok")), *cancel_btn = new wxButton(buttonpanel, wxID_CANCEL, _("Cancel"));
            buttonsizer->Add(ok_btn, 0, wxRIGHT, 5);
            buttonsizer->Add(cancel_btn);
            ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BreakpointDialog::Ok, this);
            name_textctrl->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);
            addr_textctrl->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);
            length_textctrl->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);
            exec_cbox->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);
            read_cbox->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);
            write_cbox->Bind(wxEVT_KEY_DOWN, &BreakpointDialog::OkChar, this);

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
        void OkChar(wxKeyEvent &evt)
        {
            int code = evt.GetKeyCode();
            if (code == WXK_RETURN || code == WXK_NUMPAD_ENTER)
                EndModal(wxOK);
            else
                evt.Skip();
        }

        void GetValues(wxString *name, uint32_t *address, uint32_t *length, uint32_t *type)
        {
            // type is uint32_t instead of char just to make code look nicer :p
            *name = name_textctrl->GetValue();
            *address = strtoul(addr_textctrl->GetValue(), 0, 16);
            *length = strtoul(length_textctrl->GetValue(), 0, 16);
            *type = 0;
            if (exec_cbox->GetValue())
                *type |= BREAK_TYPE_EXECUTE;
            if (read_cbox->GetValue())
                *type |= BREAK_TYPE_READ;
            if (write_cbox->GetValue())
                *type |= BREAK_TYPE_WRITE;
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
        Breakpoint *GenerateBreakpoint()
        {
            wxString name;
            uint32_t address, length, type;
            GetValues(&name, &address, &length, &type);
            return new Breakpoint(name, address, length, type);
        }
        BreakValidity IsValid()
        {
            wxString name;
            uint32_t address, length, type;
            GetValues(&name, &address, &length, &type);
            Breakpoint tmp_bpt(name, address, length, type);
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


class BreakDataModel : public DataViewTreeListModel
{
    public:
        BreakDataModel() : DataViewTreeListModel(4)
        {
        }
        wxString GetColumnType(unsigned int col) const
        {
            switch (col)
            {
                default:
                case name_col:
                    return "string";
                case address_col:
                case value_col:
                    return "long";
                case enabled_col:
                    return "bool";
            }
        }
        wxString GetBreakValue(Breakpoint *bpt) const
        {
            switch (bpt->GetLength())
            {
                case 1:
                    return wxString::Format("%02X",  MemRead8(bpt->GetAddress()));
                break;
                case 2:
                    return wxString::Format("%04X",  MemRead16(bpt->GetAddress()));
                break;
                case 4:
                    return wxString::Format("%08X", MemRead32(bpt->GetAddress()));
                break;
                case 8:
                {
                    uint64_t value = MemRead64(bpt->GetAddress());
                    return wxString::Format("%08X%08X",  (uint32_t)(value >> 32), (uint32_t)(value & 0xffffffff));
                }
                break;
            }
            return "";
        }
        void GetValue(wxVariant &variant, const wxDataViewItem &item_, unsigned int col) const
        {
            dvtlModelItem *item = (dvtlModelItem *)item_.GetID();
            if (item->isGroup)
            {
                if (col == 0 && item->val)
                    variant = (((dvtlGroup *)item->val)->name);
                return;
            }
            Breakpoint *bpt = (Breakpoint *)item->val;
            switch (col)
            {
                case name_col:
                    variant = bpt->GetName();
                break;
                case address_col:
                    variant = wxString::Format("%08X:%X", bpt->GetAddress(), bpt->GetLength());
                break;
                case value_col:
                    variant = GetBreakValue(bpt);
                break;
                case enabled_col:
                    variant = bpt->IsEnabled();
                break;
                default:
                break;
            }
        }
        bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
        {
            return false;
        }
        void UpdateBreakpoint(Breakpoint *bpt)
        {
            ItemChanged(wxDataViewItem(FindItem(bpt)));
        }
};


BreakpointPanel::BreakpointPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    filter = BREAK_FILTER_ALL;

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    list = new wxDataViewCtrl(this, -1, wxDefaultPosition, wxDefaultSize, wxDV_VERT_RULES | wxDV_MULTIPLE);
    name_column = list->AppendTextColumn(_("Name"), name_col, wxDATAVIEW_CELL_EDITABLE, -1, wxALIGN_NOT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    address_column = list->AppendTextColumn(_("Address"), address_col, wxDATAVIEW_CELL_EDITABLE, -1, wxALIGN_NOT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    value_column = list->AppendTextColumn(_("Value"), value_col, wxDATAVIEW_CELL_EDITABLE, -1, wxALIGN_NOT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
    enabled_column = list->AppendToggleColumn(_("Enabled"), enabled_col, wxDATAVIEW_CELL_INERT , -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    list->SetExpanderColumn(name_column);

    data_model = new BreakDataModel();
    list->AssociateModel(data_model);
    data_model->DecRef();

    sizer->Add(list, 1, wxEXPAND);
    SetSizer(sizer);


    list->Bind(wxEVT_LEFT_DOWN, &BreakpointPanel::Deselect, this);
    list->Bind(wxEVT_CONTEXT_MENU, &BreakpointPanel::RClickMenu, this);
    list->Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &BreakpointPanel::RClickItem, this);

    CreateList();
}

BreakpointPanel::~BreakpointPanel()
{

}

bool BreakpointPanel::FilterTest(Breakpoint *bpt)
{
    bool enabled = bpt->IsEnabled();
    int type = bpt->GetType();
    if ((!enabled && !(filter & BREAK_FILTER_DISABLED)) || (enabled && !(filter & BREAK_FILTER_ENABLED)))
        return false;
    if ((type & BREAK_TYPE_EXECUTE) && (filter & BREAK_FILTER_EXEC))
        return true;
    if ((type & BREAK_TYPE_READ) && (filter & BREAK_FILTER_READ))
        return true;
    if ((type & BREAK_TYPE_WRITE) && (filter & BREAK_FILTER_WRITE))
        return true;
    return false;
}

void BreakpointPanel::AddBreakpoint(Breakpoint *bpt)
{
    if (!FilterTest(bpt))
        return;

    data_model->AddItem(bpt);
}

void BreakpointPanel::CreateList()
{
    const BreakContainer *breakpoints = parent->GetBreakpoints();
    if (!breakpoints)
        return;

    int i = 0;
    for (auto it = breakpoints->begin(); it != breakpoints->end(); ++it, i++)
    {
        Breakpoint *bpt = it->second;
        // naah
        AddBreakpoint(bpt);
    }
    UpdateValues();
}

bool BreakpointPanel::HasValueChanged(int item, const uint64_t &new_value)
{
    wxString val_string;// = list->GetItemText(item, 2);
    if (val_string.empty())
        return true;
    return (strtoull(val_string, 0, 16) != new_value);
}

void BreakpointPanel::UpdateValues()
{

}

void BreakpointPanel::BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause reason, bool last_update)
{
    switch (reason)
    {
        case BREAK_ADDED:
            AddBreakpoint(bpt);
        break;
        case BREAK_REMOVED:
            data_model->RemoveItem(bpt);
        break;
        case BREAK_CHANGED:
            if (data_model->FindItem(bpt))
            {
                if (!FilterTest(bpt)) // no longer matches the filter
                    data_model->RemoveItem(bpt);
                else // just update
                    data_model->UpdateBreakpoint(bpt);
            }
            else // As it was not tracked, it's practically same as new breakpoint
                AddBreakpoint(bpt);

        break;
    }
}

void BreakpointPanel::Update(bool vi)
{
    UpdateValues();
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
                Breakpoint *bpt = dlg.GenerateBreakpoint();
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
                    if (!parent->AddBreakpoint(bpt))
                        delete bpt;
                }
            }
        }
        break;
        case break_edit:
        {
            BreakpointDialog dlg(this, -1, _("Edit breakpoint"));
            Breakpoint *bpt = (Breakpoint *)((dvtlModelItem *)list->GetSelection().GetID())->val;
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
                    wxString name;
                    uint32_t address, length, type;
                    dlg.GetValues(&name, &address, &length, &type);
                    parent->EditBreakpoint(bpt, name, address, length, type);
                }
            }
        }
        break;
        case break_disable:
        case break_enable:
        case break_delete:
        {
            wxDataViewItemArray items;
            list->GetSelections(items);
            int amt = items.GetCount();
            for (int i = 0; i < amt; i++)
            {
                Breakpoint *bpt = (Breakpoint *)((dvtlModelItem *)items.Item(i).GetID())->val;
                if (id == break_delete)
                    parent->DeleteBreakpoint(bpt, i == amt - 1);
                else
                    parent->EnableBreakpoint(bpt, id == break_enable, i == amt - 1);
            }
        }
        break;
        case break_f_disabled:
            filter ^= BREAK_FILTER_DISABLED;
            CreateList();
        break;
        case break_f_enabled:
            filter ^= BREAK_FILTER_ENABLED;
            CreateList();
        break;
        case break_f_exec:
            filter ^= BREAK_FILTER_EXEC;
            CreateList();
        break;
        case break_f_read:
            filter ^= BREAK_FILTER_READ;
            CreateList();
        break;
        case break_f_write:
            filter ^= BREAK_FILTER_WRITE;
            CreateList();
        break;
    }
}

void BreakpointPanel::GenerateFilterMenu(wxMenu *menu)
{
    wxMenuItem *f_disabled, *f_enabled, *f_exec, *f_read, *f_write;

    f_disabled = new wxMenuItem(menu, break_f_disabled, _("Disabled"), wxEmptyString, wxITEM_CHECK);
    menu->Append(f_disabled);
    f_disabled->Check(filter & BREAK_FILTER_DISABLED);

    f_enabled = new wxMenuItem(menu, break_f_enabled, _("Enabled"), wxEmptyString, wxITEM_CHECK);
    menu->Append(f_enabled);
    f_enabled->Check(filter & BREAK_FILTER_ENABLED);

    f_exec = new wxMenuItem(menu, break_f_exec, _("Execute"), wxEmptyString, wxITEM_CHECK);
    menu->Append(f_exec);
    f_exec->Check(filter & BREAK_FILTER_EXEC);

    f_read = new wxMenuItem(menu, break_f_read, _("Read"), wxEmptyString, wxITEM_CHECK);
    menu->Append(f_read);
    f_read->Check(filter & BREAK_FILTER_READ);

    f_write = new wxMenuItem(menu, break_f_write, _("Write"), wxEmptyString, wxITEM_CHECK);
    menu->Append(f_write);
    f_write->Check(filter & BREAK_FILTER_WRITE);
}

void BreakpointPanel::RClickItem(wxDataViewEvent &evt)
{
    wxMenu menu, *filter = new wxMenu;
    wxMenuItem *add, *edit, *remove, *disable, *enable, *separator;
    add = new wxMenuItem(&menu, break_add, _("New.."));
    menu.Append(add);
    if (list->GetSelection().GetID() != 0) // wx 2.9.3: list->GetSelectedItemsCount() == 1 or list->HasSelecion()
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

    separator = new wxMenuItem(&menu);
    menu.Append(separator);

    GenerateFilterMenu(filter);
    menu.AppendSubMenu(filter, _("Show"));

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &BreakpointPanel::RClickEvent, this, break_add, break_last - 1);

    PopupMenu(&menu);
}

void BreakpointPanel::RClickMenu(wxCommandEvent &evt)
{
    wxMenu menu, *filter = new wxMenu;
    wxMenuItem *add, *separator;
    add = new wxMenuItem(&menu, break_add, _("New.."));
    menu.Append(add);
    separator = new wxMenuItem(&menu);
    menu.Append(separator);

    GenerateFilterMenu(filter);
    menu.AppendSubMenu(filter, _("Show"));

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &BreakpointPanel::RClickEvent, this, break_add, break_last - 1);

    PopupMenu(&menu);
}


void BreakpointPanel::Deselect(wxMouseEvent &evt)
{
    list->UnselectAll();
}

