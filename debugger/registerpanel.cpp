#include "registerpanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/aui/auibook.h>
#include <wx/menu.h>

#include "../mupen64plusplus/MupenAPI.h"
#include "colors.h"

#define singlereg_height (g_textctrl_default.y + 1)


enum
{
    menu_reset = 1,
    cop0_togglereserved,
    cop1_float32,
    cop1_float64,
    cop1_raw,
};

const char *gpr_names[] =
{
    "R0", "AT", "V0", "V1", "A0", "A1", "A2", "A3", "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7",
    "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "T8", "T9", "K0", "K1", "GP", "SP", "S8", "RA"
};

const char *cop0_names[] =
{
    "Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired", "Reserved7",
    "BadVaddr", "Count", "EntryHi", "Compare", "Status", "Cause", "EPC", "PrevID",
    "Config", "LLAddr", "WatchLo", "WatchHi", "XContext", "Reserved21", "Reserved22", "Reserved23",
     "Reserved24", "Reserved25", "PErr", "CacheErr", "TagLo", "TagHi", "ErrorEPC", "Reserved31"
};

SingleRegister::SingleRegister(RegisterTab *parent_, int id, const char *name, RegisterType type_, wxPoint &pos, int reg_name_len) : wxPanel(parent_, id, pos)
{
    parent = parent_;
    type = type_;

    wxStaticText *text = new wxStaticText(this, -1, name, wxPoint(0, 1));

    if (type == REGISTER_INT64 || type == REGISTER_INT32)
    {
        value = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len, 1));
        value->SetSize(g_number_width * 10.5, -1);
        if (type == REGISTER_INT64)
        {
            value2 = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len + g_number_width * 11, 1));
            value2->SetSize(g_number_width * 10.5, -1);
            value2->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SingleRegister::ValueChanged, this);
            value2->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
            SetSize(g_number_width * 23 + reg_name_len, 28);
        }
        else
        {
            SetSize(g_number_width * 11 + reg_name_len, 28);
        }
    }
    else
    {
        value = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len, 1));
        value->SetSize(110, -1);
        SetSize(120 + reg_name_len, 28);
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
    switch (type)
    {
        case REGISTER_INT64:
        {
            uint64_t val = (uint64_t)(strtoul(value->GetValue(), 0, 16)) << 32;
            parent->ValueChanged(GetId(), val | strtoul(value2->GetValue(), 0, 16));
        }
        break;
        case REGISTER_INT32:
            parent->ValueChanged(GetId(), strtoul(value->GetValue(), 0, 16));
        break;
        case REGISTER_FLOAT:
        {
            double val;
            value->GetValue().ToCDouble(&val);
            parent->ValueChanged(GetId(), val);
        }
        break;
        default:
        break;
    }
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

void SingleRegister::SetDouble(double new_value)
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

RegisterTab::RegisterTab(wxWindow *parent, int id) : wxScrolledWindow(parent, id)
{
    rows = 1;
    cols = 1;

    Bind(wxEVT_SIZE, &RegisterTab::Size, this);
    SetScrollbars(50, singlereg_height, 1, 1);
}

RegisterTab::~RegisterTab()
{
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

    int entries = GetAmount();
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

/// ----------------------------------------------

GprTab::GprTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = 20;
    reg_basewidth = g_number_width * 23;
    raw_registers = (uint64_t *)GetRegister(M64P_CPU_REG_REG);
    for (int i = 0; i < 32; i++)
    {
        wxPoint pos = CalcItemPos(i);
        registers[i] = new SingleRegister(this, i + 1, gpr_names[i], REGISTER_INT64, pos, reg_name_len);
    }
    SetFocus();
}

GprTab::~GprTab()
{
    for (int i = 0; i < 32; i++)
        delete registers[i];
}

void GprTab::Update()
{
    for (int i = 0; i < 32; i++)
        registers[i]->SetInt(raw_registers[i]);
}

void GprTab::ValueChanged(int id, const wxAny &value)
{
    raw_registers[id - 1] = value.As<uint64_t>();
}

void GprTab::Reorder()
{
    for (uint32_t i = 0; i < 32; i++)
        registers[i]->SetPosition(CalcItemPos(i));
}

/// ----------------------------------------------

Cop0Tab::Cop0Tab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = 80;
    reg_basewidth = g_number_width * 23 - 60;
    raw_registers = (uint32_t *)GetRegister(M64P_CPU_REG_COP0);
    show_reserved = false;
    for (int i = 0; i < 32; i++)
    {
        wxPoint pos = CalcItemPos(i);
        registers[i] = new SingleRegister(this, i + 1, cop0_names[i], REGISTER_INT32, pos, reg_name_len);
    }
}

Cop0Tab::~Cop0Tab()
{
    for (int i = 0; i < 32; i++)
        delete registers[i];
    SetFocus();
}

void Cop0Tab::Update()
{
    for (int i = 0; i < 32; i++)
        registers[i]->SetInt(raw_registers[i]);
}

void Cop0Tab::ValueChanged(int id, const wxAny &value)
{
    raw_registers[id - 1] = value.As<uint32_t>();
}

void Cop0Tab::Reorder()
{
    if (!show_reserved)
    {
        for (int i = 0; i < 7; i++)
            registers[i]->SetPosition(CalcItemPos(i));
        for (int i = 8; i < 21; i++)
            registers[i]->SetPosition(CalcItemPos(i - 1));
        for (int i = 26; i < 31; i++)
            registers[i]->SetPosition(CalcItemPos(i - 6));
    }
    else
    {
        for (uint32_t i = 0; i < 32; i++)
            registers[i]->SetPosition(CalcItemPos(i));
    }
}

int Cop0Tab::GetAmount()
{
    if (show_reserved)
        return 32;
    return 25;
}

/// ----------------------------------------------

Cop1Tab::Cop1Tab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = 30;
    reg_basewidth = g_number_width * 23 - 10;
    raw_registers_simple = (float **)GetRegister(M64P_CPU_REG_COP1_SIMPLE_PTR);
    raw_registers_double = (double **)GetRegister(M64P_CPU_REG_COP1_DOUBLE_PTR);
    raw_registers_raw = (uint64_t *)GetRegister(M64P_CPU_REG_COP1_FGR_64);
    mode = cop1_float32;
    additional_regs = (((uint32_t *)GetRegister(M64P_CPU_REG_COP0))[12] & 0x04000000);
    for (int i = 0; i < 32; i++)
    {
        char buf[8];
        sprintf(buf, "F%d", i);
        wxPoint pos = CalcItemPos(i);
        registers[i] = new SingleRegister(this, i + 1, buf, REGISTER_FLOAT, pos, reg_name_len);
    }
    SetFocus();
}

Cop1Tab::~Cop1Tab()
{
    for (int i = 0; i < 32; i++)
        delete registers[i];
}

void Cop1Tab::Update()
{
    bool new_additional_regs = (((uint32_t *)GetRegister(M64P_CPU_REG_COP0))[12] & 0x04000000);
    if (new_additional_regs != additional_regs && mode == cop1_float64)
    {
        additional_regs = new_additional_regs;
        for (int i = 0; i < 16; i++)
            registers[i * 2 + 1]->Show(additional_regs);
        Reorder();
    }
    UpdateRegs();
}

void Cop1Tab::UpdateRegs()
{
    switch (mode)
    {
        case cop1_float32:
            for (int i = 0; i < 32; i++)
                registers[i]->SetFloat(*(raw_registers_simple[i]));
        break;
        case cop1_float64:
            if (additional_regs)
            {
                for (int i = 0; i < 32; i++)
                    registers[i]->SetDouble(*(raw_registers_double[i]));
            }
            else
            {
                for (int i = 0; i < 16; i++)
                    registers[i]->SetDouble(*(raw_registers_double[i * 2]));
            }
        break;
        case cop1_raw:
            for (int i = 0; i < 32; i++)
                registers[i]->SetInt(raw_registers_raw[i]);
        break;
    }
}

void Cop1Tab::ValueChanged(int id, const wxAny &value)
{
    switch (mode)
    {
        case cop1_float32:
            *(raw_registers_simple[id - 1]) = value.As<float>();
        break;
        case cop1_float64:
            *(raw_registers_double[id - 1]) = value.As<double>();
        break;
        case cop1_raw:
            raw_registers_raw[id - 1] = value.As<uint64_t>();
        break;
    }
}

void Cop1Tab::Reorder()
{
    if (mode == cop1_float64 && additional_regs)
    {
        for (uint32_t i = 0; i < 16; i++)
        {
            registers[i * 2]->SetPosition(CalcItemPos(i));
        }
    }
    else
    {
        for (uint32_t i = 0; i < 32; i++)
            registers[i]->SetPosition(CalcItemPos(i));
    }
}

int Cop1Tab::GetAmount()
{
    if (mode == cop1_float64 && !additional_regs)
    {
        return 16;
    }
    return 32;
}

/// ----------------------------------------------

RegisterPanel::RegisterPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);

    gpr_tab = new GprTab(notebook, -1);
    cop0_tab = new Cop0Tab(notebook, -1);
    cop1_tab = new Cop1Tab(notebook, -1);

    notebook->AddPage(gpr_tab, _("GPR"));
    notebook->AddPage(cop0_tab, _("COP0"));
    notebook->AddPage(cop1_tab, _("COP1"));

    notebook->Bind(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, &RegisterPanel::TabRClick, this);

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

void RegisterPanel::TabRClick(wxAuiNotebookEvent &evt)
{
    ((RegisterTab *)notebook->GetPage(evt.GetSelection()))->RClickMenu();
}
