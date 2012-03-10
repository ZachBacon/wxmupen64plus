#include "registerpanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/notebook.h>

#include "../mupen64plusplus/MupenAPI.h"

// These are practically wanted (default..) width for the register text controls
#define singlereg_height 22

enum
{
    gpr_page = 0,
    cop0_page,
    cop1_page
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
        value->SetSize(150, -1);
        SetSize(150 + reg_name_len, 28);
    }
}

SingleRegister::~SingleRegister()
{
}

void SingleRegister::SetInt(uint64_t new_value)
{
    if (type == REGISTER_INT64)
    {
        value->SetValue(wxString::Format("%08X", (uint32_t)(new_value >> 32)));
        value2->SetValue(wxString::Format("%08X", (uint32_t)(new_value & 0xffffffff)));
    }
    else
        value->SetValue(wxString::Format("%08X", (uint32_t)(new_value)));
}

void SingleRegister::SetFloat(double new_value)
{
    value->SetValue(wxString::FromCDouble(new_value));
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



RegisterTab::RegisterTab(wxWindow *parent, int id, int reg_name_len_, int reg_basewidth_) : wxScrolledWindow(parent, id)
{
    rows = 1;
    cols = 1;
    reg_name_len = reg_name_len_;
    reg_basewidth = reg_basewidth_;
    Bind(wxEVT_SIZE, &RegisterTab::Size, this);
    SetScrollbars(50, singlereg_height, 1, 1);
}

RegisterTab::~RegisterTab()
{
}

void RegisterTab::SetInt(int index, uint64_t value)
{
    registers[index]->SetInt(value);
}

void RegisterTab::SetFloat(int index, double value)
{
    registers[index]->SetFloat(value);
}

void RegisterTab::Size(wxSizeEvent &evt)
{
    evt.Skip();

    wxSize size = evt.GetSize();
    int new_cols = size.x / (reg_basewidth + reg_name_len);
    if(!new_cols)
        new_cols = 1;
    if(new_cols == cols)
        return;

    int entries = registers.size();
    cols = new_cols;
    rows = entries / cols;
    if(entries % cols)
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
    for(uint32_t i = 0; i < registers.size(); i++)
    {
        registers[i]->SetPosition(CalcItemPos(i));
    }
}

void RegisterTab::Append(const char *name, RegisterType type)
{
    wxPoint pos = CalcItemPos(registers.size());
    SingleRegister *reg = new SingleRegister(this, -1, name, type, pos, reg_name_len);
    registers.push_back(reg);
}

/// ----------------------------------------------

RegisterPanel::RegisterPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    show_reserved = false;

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxNotebook(this, -1);

    gpr_tab = new RegisterTab(notebook, -1, 20, 130);
    for(int i = 0; i < 32; i++)
    {
        gpr_tab->Append(gpr_names[i], REGISTER_INT64);
    }

    cop0_tab = new RegisterTab(notebook, -1, 50, 80);
    for(int i = 0; i < 25; i++)
    {
        cop0_tab->Append(cop0_names[i], REGISTER_INT32);
    }

    cop1_tab = new RegisterTab(notebook, -1, 20, 150);
    for(int i = 0; i < 32; i++)
    {
        char buf[8];
        sprintf(buf, "F%d", i);
        cop1_tab->Append(buf, REGISTER_FLOAT);
    }

    notebook->InsertPage(gpr_page, gpr_tab, _("GPR"));
    notebook->InsertPage(cop0_page, cop0_tab, _("COP0"));
    notebook->InsertPage(cop1_page, cop1_tab, _("COP1"));

    sizer->Add(notebook, 1, wxEXPAND);

    SetSizer(sizer);
}

RegisterPanel::~RegisterPanel()
{
}

void RegisterPanel::UpdateGpr()
{
    uint64_t *data = (uint64_t *)GetRegister(M64P_CPU_REG_REG);
    for (int i = 0; i < 32; i++)
        gpr_tab->SetInt(i, data[i]);
}

void RegisterPanel::UpdateCop0()
{
    uint32_t *data = (uint32_t *)GetRegister(M64P_CPU_REG_COP0);
    if (!show_reserved)
    {
        for (int i = 0; i < 7; i++)
            cop0_tab->SetInt(i, data[i]);
        for (int i = 8; i < 21; i++)
            cop0_tab->SetInt(i - 1, data[i]);
        for (int i = 26; i < 31; i++)
            cop0_tab->SetInt(i - 6, data[i]);
    }
    else
    {
        for (int i = 0; i < 32; i++)
            cop0_tab->SetInt(i, data[i]);
    }
}

void RegisterPanel::UpdateCop1()
{
    double *data = (double *)GetRegister(M64P_CPU_REG_COP1_DOUBLE_PTR);
    for (int i = 0; i < 32; i++)
        cop1_tab->SetFloat(i, data[i]);
}

void RegisterPanel::Update(bool vi)
{
    if (vi)
        return;

    UpdateGpr();
    UpdateCop0();
    UpdateCop1();
}
