#include "registerpanel.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/notebook.h>



const char *gpr_names[] =
{
    "R0", "AT", "V0", "V1", "A0", "A1", "A2", "A3", "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7",
    "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "T8", "T9", "K0", "K1", "GP", "SP", "S8", "RA"
};


SingleRegister::SingleRegister(wxWindow *parent, int id, const char *name, RegisterType type_) : wxPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(this, -1, name);
    value = new wxTextCtrl(this, -1);
    sizer->Add(text, 0, wxEXPAND);
    sizer->Add(value, 0, wxEXPAND);
    SetSizer(sizer);

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

RegisterTab::RegisterTab(wxWindow *parent, int id) : wxPanel(parent, id)
{
    sizer = new wxGridSizer(1);

    SetSizer(sizer);
}

RegisterTab::~RegisterTab()
{
}

void RegisterTab::Append(const char *name, RegisterType type)
{
    SingleRegister *reg = new SingleRegister(this, -1, name, type);
    registers.push_back(reg);
    sizer->Add(reg, 0, wxEXPAND);
}

RegisterPanel::RegisterPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxNotebook(this, -1);

    gpr_tab = new RegisterTab(notebook, -1);
    for(int i = 0; i < 32; i++)
    {
        gpr_tab->Append(gpr_names[i], REGISTER_INT64);
    }
    notebook->InsertPage(0, gpr_tab, _("GPR"));

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
