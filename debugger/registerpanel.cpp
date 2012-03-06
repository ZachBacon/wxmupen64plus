#include "registerpanel.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/notebook.h>

// These are practically wanted (default..) width for the register text controls
#define singlereg_height 22
#define singlereg_basewidth 110

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
    wxStaticText *text = new wxStaticText(this, -1, name, wxPoint(0, 1));
    value = new wxTextCtrl(this, -1, "", wxPoint(reg_name_len + 0, 1));

    SetSize(singlereg_basewidth + reg_name_len, 28);

    type = type_;
}

SingleRegister::~SingleRegister()
{
}

void SingleRegister::SetInt(uint64_t new_value)
{
    value->SetValue(wxString::Format("%X%X", new_value >> 32, new_value & 0xffffffff));
}

void SingleRegister::SetFloat(double new_value)
{
    value->SetValue(wxString::Format("%f", new_value));
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



RegisterTab::RegisterTab(wxWindow *parent, int id, int reg_name_len_) : wxScrolledWindow(parent, id)
{
    rows = 1;
    cols = 1;
    reg_name_len = reg_name_len_;
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
    int new_cols = size.x / (singlereg_basewidth + reg_name_len);
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
    return wxPoint(5 + col * (singlereg_basewidth + reg_name_len), (row - scrolled) * singlereg_height);
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
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxNotebook(this, -1);

    gpr_tab = new RegisterTab(notebook, -1, 20);
    for(int i = 0; i < 32; i++)
    {
        gpr_tab->Append(gpr_names[i], REGISTER_INT64);
    }

    cop0_tab = new RegisterTab(notebook, -1, 50);
    for(int i = 0; i < 25; i++)
    {
        printf("%d\n", i);
        cop0_tab->Append(cop0_names[i], REGISTER_INT64);
    }

    cop1_tab = new RegisterTab(notebook, -1, 20);
    for(int i = 0; i < 32; i++)
    {
        char buf[8];
        sprintf(buf, "F%d", i);
        cop1_tab->Append(buf, REGISTER_FLOAT);
    }

    notebook->InsertPage(0, gpr_tab, _("GPR"));
    notebook->InsertPage(1, cop0_tab, _("COP0"));
    notebook->InsertPage(2, cop1_tab, _("COP1"));

    sizer->Add(notebook, 1, wxEXPAND);

    SetSizer(sizer);
}

RegisterPanel::~RegisterPanel()
{
}

void RegisterPanel::Update(bool vi)
{
    if(vi)
        return;
}
