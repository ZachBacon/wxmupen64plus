/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus debugger                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Markus Heikkinen                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "registerpanel.h"

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/aui/auibook.h>
#include <wx/menu.h>
#include <wx/statline.h>

#include "../mupen64plusplus/MupenAPI.h"
#include "colors.h"
#include "debugconfig.h"
#include "debuggerframe.h"

#define singlereg_height (g_textctrl_default.y + 3)


enum
{
    menu_reset = 1,
    cop0_togglereserved,
    cop1_float32,
    cop1_float64,
    cop1_raw,
    value_1,
    value_2
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

const char *vi_names[] =
{
    "Status", "Origin", "Width", "VIntr", "Current", "Burst", "VSync", "HSync",
    "Leap", "HStart", "VStart", "VBurst", "XScale", "YScale", "[Delay]"
};

const char *ai_names[] =
{
    "DRAM Addr", "Length", "Control", "Status", "DacRate", "BitRate",
    "[NextDelay]", "[NextLength]", "[CurrentDelay]", "[CurrentLength]"
};

const char *pi_names[] =
{
    "DRAM Addr", "CartAddr", "ReadLength", "WriteLength", "Status",
    "Dom1 Latency", "Dom1 PulseW", "Dom1 Page Size", "Dom1 Release",
    "Dom2 Latency", "Dom2 PulseW", "Dom2 Page Size", "Dom2 Release"
};

const char *ri_names[] =
{
    "Mode", "Config", /*CurrentLoad,*/ "Select", "Refresh", "Latency", "RError"/*, "WError"*/
};

const char *si_names[] =
{
    "DRAM Addr", "Status"
};

SingleRegister::SingleRegister(RegisterTab *parent_, int id, const char *name, RegisterType type_, const wxPoint &pos, int reg_name_len) : wxPanel(parent_, id, pos)
{
    parent = parent_;
    type = type_;
    value2 = 0;

    wxStaticText *text = new wxStaticText(this, -1, name, wxPoint(0, 1));

    if (type == REGISTER_INT64 || type == REGISTER_INT32)
    {
        value = new wxTextCtrl(this, value_1, "", wxPoint(reg_name_len, 1));
        value->SetSize(g_number_width * 10.5, -1);
        if (type == REGISTER_INT64)
        {
            value2 = new wxTextCtrl(this, value_2, "", wxPoint(reg_name_len + g_number_width * 11, 1));
            value2->SetSize(g_number_width * 10.5, -1);
            value2->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SingleRegister::ValueChanged, this);
            value2->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
        }
    }
    else
    {
        value = new wxTextCtrl(this, value_1, "", wxPoint(reg_name_len, 1));
        value->SetSize(g_number_width * 20, -1);
    }
    SetSize(g_number_width * 26, 28);
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
    int id = evt.GetId();
    if (type == REGISTER_INT64)
    {
        DebuggerFrame *frame = parent->GetParent()->GetParent(); // ^^
        if (frame->DoFollow(id, GetInt()))
            return;
    }
    switch (id)
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

    if (type == REGISTER_INT64 && evt.GetId() == value_2 && MemIsValid(GetInt()))
    {
        DebuggerFrame *frame = parent->GetParent()->GetParent(); // ^^
        menu.AppendSeparator();
        frame->MenuAppendDisasmFollow(&menu);
        frame->MenuAppendMemoryFollow(&menu);
    }

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
    if (type == REGISTER_INT64)
        return (uint64_t)strtoull(value->GetValue(), 0, 16) << 32 | strtoull(value2->GetValue(), 0, 16);
    else
        return strtoull(value->GetValue(), 0, 16);
}

double SingleRegister::GetFloat()
{
    return strtod(value->GetValue(), 0);
}

void SingleRegister::SetType(RegisterType new_type)
{
    if (type == new_type)
        return;

    if (type == REGISTER_INT64)
        value2->Hide();
    else if (type == REGISTER_FLOAT)
        value->SetSize(g_number_width * 10.5, -1);

    if (new_type == REGISTER_INT64)
    {
        if (!value2)
        {
            value2 = new wxTextCtrl(this, value_2, "", wxPoint(value->GetPosition().x + g_number_width * 11, 1));
            value2->SetSize(g_number_width * 10.5, -1);
            value2->Bind(wxEVT_COMMAND_TEXT_UPDATED, &SingleRegister::ValueChanged, this);
            value2->Bind(wxEVT_RIGHT_UP, &SingleRegister::RClickMenu, this);
        }
        else
            value2->Show();
    }
    else if (new_type == REGISTER_FLOAT)
        value->SetSize(g_number_width * 20, -1);
    type = new_type;
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

    Resize(new_cols);

    SetSize(size);
}

void RegisterTab::Resize(int new_cols)
{
    int entries = GetAmount();
    cols = new_cols;
    rows = entries / cols;
    if (entries % cols)
        rows += 1;

    SetVirtualSize(0, 5 + singlereg_height * (rows - 1));

    Reorder();
}

wxPoint RegisterTab::CalcItemPos(int index)
{
    int col = index / rows, row = index % rows;
    int scrolled = GetViewStart().y;
    return wxPoint(5 + col * (reg_basewidth + reg_name_len), (row - scrolled) * singlereg_height + 2);
}

/// ----------------------------------------------

GprTab::GprTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 3;
    reg_basewidth = g_number_width * 23;
    raw_registers = (uint64_t *)GetRegister(M64P_CPU_REG_REG);
    raw_hi = (uint64_t *)GetRegister(M64P_CPU_REG_HI);
    raw_lo = (uint64_t *)GetRegister(M64P_CPU_REG_LO);
    for (int i = 0; i < 32; i++)
        registers[i] = new SingleRegister(this, i + 1, gpr_names[i], REGISTER_INT64, CalcItemPos(i), reg_name_len);

    int rows = GetRows();
    separator = new wxStaticLine(this, -1, CalcItemPos(rows - 1) + wxPoint(g_number_width * 3, singlereg_height + 4), wxSize(g_number_width * (26 * GetCols() - 6), -1));
    hi = new SingleRegister(this, 33, "HI", REGISTER_INT64, CalcItemPos(rows - 1) + wxPoint(0, singlereg_height + 7), reg_name_len);
    if (GetCols() < 2)
        lo = new SingleRegister(this, 34, "LO", REGISTER_INT64, CalcItemPos(rows - 1) + wxPoint(0, singlereg_height * 2 + 7), reg_name_len);
    else
        lo = new SingleRegister(this, 34, "LO", REGISTER_INT64, CalcItemPos(rows * 2 - 1) + wxPoint(0, singlereg_height + 7), reg_name_len);
    SetFocus();
}

GprTab::~GprTab()
{
}

void GprTab::Update()
{
    for (int i = 0; i < 32; i++)
        registers[i]->SetInt(raw_registers[i]);
    hi->SetInt(*raw_hi);
    lo->SetInt(*raw_lo);
}

void GprTab::ValueChanged(int id, const wxAny &value)
{
    if (id <= 32)
        raw_registers[id - 1] = value.As<uint64_t>();
    else if (id == 33)
        *raw_hi = value.As<uint64_t>();
    else if (id == 34)
        *raw_lo = value.As<uint64_t>();
}

void GprTab::Reorder()
{
    for (uint32_t i = 0; i < 32; i++)
        registers[i]->SetPosition(CalcItemPos(i));
    int rows = GetRows();
    separator->SetPosition(CalcItemPos(rows - 1) + wxPoint(g_number_width * 3, singlereg_height + 4));
    separator->SetSize(wxSize(g_number_width * (26 * GetCols() - 6), -1));
    hi->SetPosition(CalcItemPos(rows - 1) + wxPoint(0, singlereg_height + 9));
    if (GetCols() < 2)
        lo->SetPosition(CalcItemPos(rows - 1) + wxPoint(0, singlereg_height * 2 + 9));
    else
        lo->SetPosition(CalcItemPos(rows * 2 - 1) + wxPoint(0, singlereg_height + 9));

    Refresh();

    if (GetCols() < 2) // sigh sigh sigh
        SetVirtualSize(0, 12 + singlereg_height * (rows + 1));
    else
        SetVirtualSize(0, 12 + singlereg_height * rows);
}

/// ----------------------------------------------

Cop0Tab::Cop0Tab(wxWindow *parent, int id, int config) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 13;
    reg_basewidth = g_number_width * 13;
    raw_registers = (uint32_t *)GetRegister(M64P_CPU_REG_COP0);
    show_reserved = config & 0x1;
    for (int i = 0; i < 32; i++)
    {
        wxPoint pos = CalcItemPos(i);
        registers[i] = new SingleRegister(this, i + 1, cop0_names[i], REGISTER_INT32, pos, reg_name_len);
    }
}

Cop0Tab::~Cop0Tab()
{
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

void Cop0Tab::RClickMenu()
{
    wxMenu menu;

    wxMenuItem *toggle_reserved = new wxMenuItem(&menu, cop0_togglereserved, _("Show reserved registers"), wxEmptyString, wxITEM_CHECK);
    menu.Append(toggle_reserved);
    toggle_reserved->Check(show_reserved);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &Cop0Tab::RClickEvent, this);
    PopupMenu(&menu);
}

void Cop0Tab::RClickEvent(wxCommandEvent &evt)
{
    switch(evt.GetId())
    {
        case cop0_togglereserved:
            if (show_reserved)
                show_reserved = false;
            else
                show_reserved = true;

            registers[7]->Show(show_reserved);
            for (int i = 21; i < 26; i++)
                registers[i]->Show(show_reserved);
            registers[31]->Show(show_reserved);

            Resize(GetCols());
            Refresh();
        break;
    }
}

/// ----------------------------------------------

Cop1Tab::Cop1Tab(wxWindow *parent, int id, int config) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 4;
    reg_basewidth = g_number_width * 22;
    raw_registers_simple = (float **)GetRegister(M64P_CPU_REG_COP1_SIMPLE_PTR);
    raw_registers_double = (double **)GetRegister(M64P_CPU_REG_COP1_DOUBLE_PTR);
    raw_registers_raw = (uint64_t *)GetRegister(M64P_CPU_REG_COP1_FGR_64);
    mode = config;
    RegisterType reg_type;
    (mode == cop1_raw) ? (reg_type = REGISTER_INT64) : (reg_type = REGISTER_FLOAT);
    additional_regs = (((uint32_t *)GetRegister(M64P_CPU_REG_COP0))[12] & 0x04000000);

    for (int i = 0; i < 32; i++)
    {
        char buf[8];
        sprintf(buf, "F%d", i);
        wxPoint pos = CalcItemPos(i);
        registers[i] = new SingleRegister(this, i + 1, buf, reg_type, pos, reg_name_len);
    }
    SetFocus();
}

Cop1Tab::~Cop1Tab()
{
}

void Cop1Tab::Update()
{
    bool new_additional_regs = (((uint32_t *)GetRegister(M64P_CPU_REG_COP0))[12] & 0x04000000);
    if (new_additional_regs != additional_regs)
    {
        additional_regs = new_additional_regs;
        if (mode == cop1_float64)
        {
            for (int i = 0; i < 16; i++)
                registers[i * 2 + 1]->Show(additional_regs);
            Reorder();
            Refresh();
        }
        else if (mode == cop1_raw)
        {
            for (int i = 16; i < 32; i++)
                registers[i]->Enable(additional_regs == true);
        }
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
                    registers[i * 2]->SetDouble(*(raw_registers_double[i * 2]));
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
    for (uint32_t i = 0; i < 32; i++)
        registers[i]->SetPosition(CalcItemPos(i));
}

int Cop1Tab::GetAmount()
{
    if (mode == cop1_float64 && !additional_regs)
    {
        return 16;
    }
    return 32;
}

void Cop1Tab::RClickMenu()
{
    wxMenu menu;

    wxMenuItem *f32 = new wxMenuItem(&menu, cop1_float32, _("Show as 32-bit floats"), wxEmptyString, wxITEM_CHECK);
    menu.Append(f32);
    wxMenuItem *f64 = new wxMenuItem(&menu, cop1_float64, _("Show as 64-bit floats"), wxEmptyString, wxITEM_CHECK);
    menu.Append(f64);
    wxMenuItem *raw = new wxMenuItem(&menu, cop1_raw, _("Show as 64-bit integers"), wxEmptyString, wxITEM_CHECK);
    menu.Append(raw);
    switch (mode)
    {
        case cop1_float32:
            f32->Check(true);
        break;
        case cop1_float64:
            f64->Check(true);
        break;
        case cop1_raw:
            raw->Check(true);
        break;
    }

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &Cop1Tab::RClickEvent, this);
    PopupMenu(&menu);
}

void Cop1Tab::RClickEvent(wxCommandEvent &evt)
{
    int new_mode = evt.GetId();
    if (mode == new_mode)
        return;

    Freeze();
    switch (mode)
    {
        case cop1_float64:
            for (int i = 0; i < 16; i++)
                registers[i * 2 + 1]->Show(true);
        break;
        case cop1_raw:
            for (int i = 0; i < 32; i++)
                registers[i]->SetType(REGISTER_FLOAT);
            for (int i = 16; i < 32; i++)
                registers[i]->Enable(true);
        break;
    }
    switch (new_mode)
    {
        case cop1_float64:
            for (int i = 0; i < 16; i++)
                registers[i * 2 + 1]->Show(additional_regs == true);
        break;
        case cop1_raw:
            for (int i = 0; i < 32; i++)
                registers[i]->SetType(REGISTER_INT64);
            for (int i = 16; i < 32; i++)
                registers[i]->Enable(additional_regs == true);
        break;
    }
    mode = new_mode;
    Refresh();
    Reorder();
    UpdateRegs();
    Thaw();
}

/// ----------------------------------------------

ViTab::ViTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 13;
    reg_basewidth = g_number_width * 13;
    raw_registers = (uint32_t *)GetMemoryPointer(M64P_DBG_PTR_VI_REG);
    for (int i = 0; i < 15; i++)
    {
        registers[i] = new SingleRegister(this, i + 1, vi_names[i], REGISTER_INT32, CalcItemPos(i), reg_name_len);
    }
    SetFocus();
}

ViTab::~ViTab()
{
}

void ViTab::Update()
{
    for (int i = 0; i < 15; i++)
        registers[i]->SetInt(raw_registers[i]);
}

void ViTab::ValueChanged(int id, const wxAny &value)
{
    raw_registers[id - 1] = value.As<uint32_t>();
}

void ViTab::Reorder()
{
    for (uint32_t i = 0; i < 15; i++)
        registers[i]->SetPosition(CalcItemPos(i));
}

/// ----------------------------------------------

AiTab::AiTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 13;
    reg_basewidth = g_number_width * 13;
    raw_registers = (uint32_t *)GetMemoryPointer(M64P_DBG_PTR_AI_REG);
    for (int i = 0; i < 10; i++)
    {
        registers[i] = new SingleRegister(this, i + 1, ai_names[i], REGISTER_INT32, CalcItemPos(i), reg_name_len);
    }
    SetFocus();
}

AiTab::~AiTab()
{
}

void AiTab::Update()
{
    for (int i = 0; i < 10; i++)
        registers[i]->SetInt(raw_registers[i]);
}

void AiTab::ValueChanged(int id, const wxAny &value)
{
    raw_registers[id - 1] = value.As<uint32_t>();
}

void AiTab::Reorder()
{
    for (uint32_t i = 0; i < 10; i++)
        registers[i]->SetPosition(CalcItemPos(i));
}

/// ----------------------------------------------

PiTab::PiTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 13;
    reg_basewidth = g_number_width * 13;
    raw_registers = (uint32_t *)GetMemoryPointer(M64P_DBG_PTR_PI_REG);
    for (int i = 0; i < 13; i++)
    {
        registers[i] = new SingleRegister(this, i + 1, pi_names[i], REGISTER_INT32, CalcItemPos(i), reg_name_len);
    }
    SetFocus();
}

PiTab::~PiTab()
{
}

void PiTab::Update()
{
    for (int i = 0; i < 13; i++)
        registers[i]->SetInt(raw_registers[i]);
}

void PiTab::ValueChanged(int id, const wxAny &value)
{
    raw_registers[id - 1] = value.As<uint32_t>();
}

void PiTab::Reorder()
{
    for (uint32_t i = 0; i < 13; i++)
        registers[i]->SetPosition(CalcItemPos(i));
}

/// ----------------------------------------------

RiSiTab::RiSiTab(wxWindow *parent, int id) : RegisterTab(parent, id)
{
    reg_name_len = g_number_width * 13;
    reg_basewidth = g_number_width * 13;
    raw_ri_registers = (uint32_t *)GetMemoryPointer(M64P_DBG_PTR_RI_REG);
    raw_si_registers = (uint32_t *)GetMemoryPointer(M64P_DBG_PTR_SI_REG);
    ri_text = new wxStaticText(this, -1, _("RDRAM Interface"), CalcTextPos(0));
    for (int i = 0; i < 6; i++)
        registers[i] = new SingleRegister(this, i + 1, ri_names[i], REGISTER_INT32, CalcItemPos(i), reg_name_len);
    si_text = new wxStaticText(this, -1, _("Serial Interface"), CalcTextPos(1));
    for (int i = 6; i < 8; i++)
        registers[i] = new SingleRegister(this, i + 1, si_names[i - 6], REGISTER_INT32, CalcItemPos(i), reg_name_len);
    SetFocus();
}

RiSiTab::~RiSiTab()
{
}

void RiSiTab::Update()
{
    for (int i = 0; i < 6; i++)
        registers[i]->SetInt(raw_ri_registers[i]);
    registers[6]->SetInt(raw_si_registers[0]);
    registers[7]->SetInt(raw_si_registers[3]);
}

void RiSiTab::ValueChanged(int id, const wxAny &value)
{
    if (id == 7)
        raw_si_registers[0] = value.As<uint32_t>();
    else if (id == 8)
        raw_si_registers[3] = value.As<uint32_t>();
    else
        raw_ri_registers[id - 1] = value.As<uint32_t>();
}

void RiSiTab::Reorder()
{
    ri_text->SetPosition(CalcTextPos(0));
    si_text->SetPosition(CalcTextPos(1));
    for (uint32_t i = 0; i < 6; i++)
        registers[i]->SetPosition(CalcItemPos(i));
    for (uint32_t i = 6; i < 8; i++)
        registers[i]->SetPosition(CalcItemPos(i));

    SetVirtualSize(0, 5 + singlereg_height * GetRows());
    Refresh();
}

wxPoint RiSiTab::CalcTextPos(int index) // this stuff makes sense
{
    int scrolled = GetViewStart().y;
    if (index == 0)
        return wxPoint(5, (0 - scrolled) * singlereg_height + 2);

    int cols = GetCols();
    int row;
    if (6 % cols)
        row = 2 + 6 / cols;
    else
        row = 1 + 6 / cols;

    return wxPoint(5, (row - scrolled) * singlereg_height + 2);
}

wxPoint RiSiTab::CalcItemPos(int index)
{
    int scrolled = GetViewStart().y;
    int cols = GetCols();
    if (cols > 3)
        cols = 3;
    int col, row;
    if (index >= 6)
    {
        index -= 6;
        row = index % (6 / cols) + 2 + (6 / cols);
    }
    else
        row = index % (6 / cols) + 1;

    col = index / (6 / cols);


    return wxPoint(5 + col * (reg_basewidth + reg_name_len), (row - scrolled) * singlereg_height + 2);
}

/// ----------------------------------------------

RegisterPanel::RegisterPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config) : DebugPanel(parent, id, type)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    notebook = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP);

    notebook->AddPage(new GprTab(notebook, -1), _("GPR"));
    notebook->AddPage(cop0tab = new Cop0Tab(notebook, -1, atoi(config.GetValue("Cop0", "0"))), _("COP0"));
    notebook->AddPage(cop1tab = new Cop1Tab(notebook, -1, atoi(config.GetValue("Cop1", "0"))), _("COP1"));
    notebook->AddPage(new ViTab(notebook, -1), _("VI"));
    notebook->AddPage(new AiTab(notebook, -1), _("AI"));
    notebook->AddPage(new PiTab(notebook, -1), _("PI"));
    notebook->AddPage(new RiSiTab(notebook, -1), _("RI&SI"));

    notebook->SetSelection(atoi(config.GetValue("Tab", "0")));

    updated_panels = 0;

    notebook->Bind(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, &RegisterPanel::TabRClick, this);

    sizer->Add(notebook, 1, wxEXPAND);
    SetSizer(sizer);
}

RegisterPanel::~RegisterPanel()
{
}

void RegisterPanel::SaveConfig(DebugConfigOut &config, DebugConfigSection &section)
{
    section.num_values = 3;
    char tab_buf[8], c0_buf[8], c1_buf[8];

    sprintf(tab_buf, "%d", notebook->GetSelection());
    sprintf(c0_buf, "%d", cop0tab->GetConfig());
    sprintf(c1_buf, "%d", cop1tab->GetConfig());
    section.keys[0] = "Tab";
    section.values[0] = tab_buf;
    section.keys[1] = "Cop0";
    section.values[1] = c0_buf;
    section.keys[2] = "Cop1";
    section.values[2] = c1_buf;
    config.WriteSection(section);
}


void RegisterPanel::Update(bool vi)
{
    int pages = notebook->GetPageCount();
    for (int i = 0; i < pages; i++)
    {
        if (vi && i == 0) // It's pointless to refresh GRP tab
            continue;
        RegisterTab *page = (RegisterTab *)notebook->GetPage(i);
        page->Update();
    }
}

void RegisterPanel::TabRClick(wxAuiNotebookEvent &evt)
{
    ((RegisterTab *)notebook->GetPage(evt.GetSelection()))->RClickMenu();
}


