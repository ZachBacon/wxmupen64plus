#include "registerpanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/aui/auibook.h>
#include <wx/menu.h>

#include "../mupen64plusplus/MupenAPI.h"
#include "colors.h"

#define singlereg_height (g_textctrl_default.y + 1)

wxDEFINE_EVENT(REGVAL_CHANGE_EVT, RegChangeEvent);

enum
{
    menu_reset = 1,
    cop0_togglereserved,
    cop1_float32,
    cop1_float64,
    cop1_raw
};

const char *gpr_names[] =
{
    "R0", "AT", "V0", "V1", "A0", "A1", "A2", "A3", "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7",
    "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "T8", "T9", "K0", "K1", "GP", "SP", "S8", "RA"
};

const char *cop0_names[] =
{
    "Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired", /*"Reserved",*/
    "BadVaddr", "Count", "EntryHi", "Compare", "Status", "Cause", "EPC", "PrevID",
    "Config", "LLAddr", "WatchLo", "WatchHi", "XContext", /*"Reserved",*/ /*"Reserved",*/ /*"Reserved",*/
     /*"Reserved",*/ /*"Reserved",*/ "PErr", "CacheErr", "TagLo", "TagHi", "ErrorEPC" /*", Reserved"*/
};

SingleRegister::SingleRegister(wxWindow *parent, int id, const char *name, RegisterType type_, wxPoint &pos, int reg_name_len) : wxPanel(parent, id, pos)
{
    type = type_;

    wxStaticText *text = new wxStaticText(this, -1, name, wxPoint(0, 1));

    if (type == REGISTER_INT64 || type == REGISTER_INT32)
    {
        value = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len, 1));
        value->SetSize(60, -1);
        if (type == REGISTER_INT64)
        {
            value2 = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len + 62, 1));
            value2->SetSize(60, -1);
            value2->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SingleRegister::ValueChanged, this);
            value2->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
            SetSize(130 + reg_name_len, 28);
        }
        else
        {
            SetSize(65 + reg_name_len, 28);
        }
    }
    else
    {
        value = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len, 1));
        value->SetSize(120, -1);
        SetSize(130 + reg_name_len, 28);
    }
    value->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SingleRegister::ValueChanged, this);
    value->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
    text->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
    Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
}

SingleRegister::~SingleRegister()
{
}

void SingleRegister::RClickEvent(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case menu_reset:
            ResetValue();
        break;
        default:
        break;
    }
}

void SingleRegister::RClickMenu(wxMouseEvent &evt)
{
    wxMenu menu;
    wxMenuItem *reset;

    reset = new wxMenuItem(&menu, menu_reset, _("Reset value"));
    menu.Append(reset);
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &SingleRegister::RClickEvent, this);
    PopupMenu(&menu);
}

void SingleRegister::ValueChanged(wxCommandEvent &evt)
{
    if (setting_value)
        return;
    ValueChanged();
}

void SingleRegister::ValueChanged()
{
    RegChangeEvent evt(GetId());
    switch (type)
    {
        case REGISTER_INT64:
        {
            uint64_t val = (uint64_t)(strtoul(value->GetValue(), 0, 16)) << 32;
            val |= strtoul(value2->GetValue(), 0, 16);
            evt.value = val;
        }
        break;
        case REGISTER_INT32:
            evt.value = strtoul(value->GetValue(), 0, 16);
        break;
        case REGISTER_FLOAT:
        {
            double val;
            value->GetValue().ToCDouble(&val);
            evt.value = val;
        }
        break;
        default:
        break;
    }
    ProcessEvent(evt);
}

void SingleRegister::SetInt(uint64_t new_value)
{
    setting_value = true;
    original_value = new_value;
    if (type == REGISTER_INT64)
    {
        value->SetValue(wxString::Format("%08X", (uint32_t)(new_value >> 32)));
        value2->SetValue(wxString::Format("%08X", (uint32_t)(new_value & 0xffffffff)));
    }
    else
        value->SetValue(wxString::Format("%08X", (uint32_t)(new_value)));
    setting_value = false;
}

void SingleRegister::SetFloat(float new_value)
{
    setting_value = true;
    original_value = new_value;
    value->SetValue(wxString::FromCDouble(new_value));
    setting_value = false;
}

void SingleRegister::ResetValue()
{
    switch (type)
    {
        case REGISTER_INT64:
            value2->SetValue(wxString::Format("%08X", (uint32_t)(original_value.As<uint64_t>() & 0xffffffff)));
            value->SetValue(wxString::Format("%08X", (uint32_t)(original_value.As<uint64_t>() >> 32)));
        break;
        case REGISTER_INT32:
            value->SetValue(wxString::Format("%08X", (uint32_t)(original_value.As<uint32_t>())));
        break;
        case REGISTER_FLOAT:
            value->SetValue(wxString::FromCDouble(original_value.As<float>()));
        break;
        default:
        break;
    }
    ValueChanged();
}

uint64_t SingleRegister::GetInt()
{
    return strtoull(value->GetValue(), 0, 16);
}

double SingleRegister::GetFloat()
{
    return strtod(value->GetValue(), 0);
}

/// ----------------------------------------------

RegisterTab::RegisterTab(wxWindow *parent, int id, RegisterGroup type_) : wxScrolledWindow(parent, id)
{
    rows = 1;
    cols = 1;
    InitRegisters(type_);

    Bind(wxEVT_SIZE, &RegisterTab::Size, this);
    SetScrollbars(50, singlereg_height, 1, 1);
}

void RegisterTab::InitRegisters(RegisterGroup type_)
{
    type = type_;
    switch (type)
    {
        case REGISTER_GPR:
            reg_name_len = 20;
            reg_basewidth = 130;
            raw_registers.v = GetRegister(M64P_CPU_REG_REG);
            for (int i = 0; i < 32; i++)
            {
                Append(gpr_names[i], REGISTER_INT64);
            }
        break;
        case REGISTER_COP0:
            show_reserved = false;
            reg_name_len = 50;
            reg_basewidth = 80;
            raw_registers.v = GetRegister(M64P_CPU_REG_COP0);
            for (int i = 0; i < 7; i++)
                Append(cop0_names[i], REGISTER_INT32, i + 1);
            for (int i = 8; i < 21; i++)
                Append(cop0_names[i - 1], REGISTER_INT32, i + 1);
            for (int i = 26; i < 31; i++)
                Append(cop0_names[i - 6], REGISTER_INT32, i + 1);

        break;
        case REGISTER_COP1:
            reg_name_len = 20;
            reg_basewidth = 130;
            raw_registers.v = GetRegister(M64P_CPU_REG_COP1_SIMPLE_PTR);
            for (int i = 0; i < 32; i++)
            {
                char buf[8];
                sprintf(buf, "F%d", i);
                Append(buf, REGISTER_FLOAT);
            }
        break;
        default:
        break;
    }
    SetFocus();
}

void RegisterTab::Update()
{
    switch (type)
    {
        case REGISTER_GPR:
            for (int i = 0; i < 32; i++)
                SetInt(i, raw_registers.i64[i]);
        break;
        case REGISTER_COP0:
            if (!show_reserved)
            {
                for (int i = 0; i < 7; i++)
                    SetInt(i, raw_registers.i32[i]);
                for (int i = 8; i < 21; i++)
                    SetInt(i - 1, raw_registers.i32[i]);
                for (int i = 26; i < 31; i++)
                    SetInt(i - 6, raw_registers.i32[i]);
            }
            else
            {
                for (int i = 0; i < 32; i++)
                    SetInt(i, raw_registers.i32[i]);
            }
        break;
        case REGISTER_COP1:
            for (int i = 0; i < 32; i++)
                SetFloat(i, *(raw_registers.fp[i]));
        break;
        default:
        break;
    }
}

RegisterTab::~RegisterTab()
{
}

void RegisterTab::SetInt(int index, uint64_t value)
{
    registers[index]->SetInt(value);
}

void RegisterTab::SetFloat(int index, float value)
{
    registers[index]->SetFloat(value);
}

void RegisterTab::Size(wxSizeEvent &evt)
{
    evt.Skip();

    wxSize size = evt.GetSize();
    int new_cols = size.x / (reg_basewidth + reg_name_len);
    if (!new_cols)
        new_cols = 1;
    if (new_cols == cols)
        return;

    int entries = registers.size();
    cols = new_cols;
    rows = entries / cols;
    if (entries % cols)
        rows += 1;

    SetVirtualSize(0, 5 + singlereg_height * (rows - 1));
    SetSize(size);

    Reorder();
}

wxPoint RegisterTab::CalcItemPos(int index)
{
    int col = index / rows, row = index % rows;
    int scrolled = GetViewStart().y;
    return wxPoint(5 + col * (reg_basewidth + reg_name_len), (row - scrolled) * singlereg_height);
}

void RegisterTab::Reorder()
{
    for (uint32_t i = 0; i < registers.size(); i++)
    {
        registers[i]->SetPosition(CalcItemPos(i));
    }
}

void RegisterTab::Append(const char *name, RegisterType type, int id)
{
    wxPoint pos = CalcItemPos(registers.size());
    if (id == -1)
        id = registers.size() + 1;
    SingleRegister *reg = new SingleRegister(this, id, name, type, pos, reg_name_len);
    reg->Bind(REGVAL_CHANGE_EVT, &RegisterTab::ValueChanged, this);
    registers.push_back(reg);
}

void RegisterTab::ValueChanged(RegChangeEvent &evt)
{
    int reg = evt.GetId() - 1;
    switch (type)
    {
        case REGISTER_GPR:
            raw_registers.i64[reg] = evt.value.As<uint64_t>();
        break;
        case REGISTER_COP0:
            raw_registers.i32[reg] = evt.value.As<uint32_t>();
        break;
        case REGISTER_COP1:
            *(raw_registers.fp[reg]) = evt.value.As<float>();
        break;
        default:
        break;
    }
}

/// ----------------------------------------------

RegisterPanel::RegisterPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);

    gpr_tab = new RegisterTab(notebook, -1, REGISTER_GPR);
    cop0_tab = new RegisterTab(notebook, -1, REGISTER_COP0);
    cop1_tab = new RegisterTab(notebook, -1, REGISTER_COP1);

    notebook->AddPage(gpr_tab, _("GPR"));
    notebook->AddPage(cop0_tab, _("COP0"));
    notebook->AddPage(cop1_tab, _("COP1"));

    sizer->Add(notebook, 1, wxEXPAND);
    SetSizer(sizer);
}

RegisterPanel::~RegisterPanel()
{
}

void RegisterPanel::Update(bool vi)
{
    if (vi)
        return;

    gpr_tab->Update();
    cop0_tab->Update();
    cop1_tab->Update();
}
