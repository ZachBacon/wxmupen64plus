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

#include "disasmpanel.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/scrolbar.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/clipbrd.h>
#include <wx/menu.h>

#include "debuggerframe.h"
#include "breakpoint.h"
#include "colors.h"
#include "debugconfig.h"
#include "../mupen64plusplus/MupenAPI.h"

#ifdef DrawText
#undef DrawText // windows api ftw
#endif

#define line_start_y 1
#define address_start_x 5
#define address_width (g_number_width * 8 + 10)
#define opcode_start_x (address_start_x * 2 + address_width)
#define comment_start_x (opcode_start_x + 500)
#define line_height (g_normal_height - 2)

#define scrollbar_size 0x80c // Maybe these could be member vars, changing when resized?
#define scrollbar_thumb 10 // Though now they have nice 0x4, 0x100 and (almost) 0x1000 scroll steps
#define scrollbar_thumb_defpos (scrollbar_size - scrollbar_thumb) / 2
#define scroll_multiplier 1

enum
{
    menu_id_copyins = 2,
    menu_id_copyaddr,
    menu_id_break,
};

DisasmPanel::DisasmPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config) : DebugPanel(parent, id, type)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL), *subsizer = new wxBoxSizer(wxVERTICAL);
    address = 0;
    data_lines = 0;
    data = 0;
    data_strings = 0;
    scroll_accum_delta = 0;

    code = new DisasmWindow(this, -1);

    scrollbar = new wxScrollBar(this, -1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    scrollbar->SetScrollbar(scrollbar_thumb_defpos, scrollbar_thumb, scrollbar_size, 0x40);
    scroll_thumbpos = scrollbar_thumb_defpos;

    wxPanel *subpanel = new wxPanel(this, -1);
    go_address = new wxTextCtrl(subpanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    go_address->SetMaxLength(10);
    go_address->SetMinSize(wxSize(70, -1));
    go_button = new wxButton(subpanel, -1, _("Follow"));
    wxStaticText *pc_text = new wxStaticText(subpanel, -1, _("PC"));
    pc_display = new wxTextCtrl(subpanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    pc_display->SetMinSize(wxSize(70, -1));
    pc_go = new wxButton(subpanel, -1, _("Follow PC"));
    wxStaticText *old_pc_text = new wxStaticText(subpanel, -1, _("Previous PC"));
    old_pc = new wxTextCtrl(subpanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    old_pc->SetMinSize(wxSize(70, -1));
    run = new wxButton(subpanel, -1, _("Run"));
    pause = new wxButton(subpanel, -1, _("Pause"));
    step = new wxButton(subpanel, -1, _("Step"));
    wxStaticLine *separator_line = new wxStaticLine(subpanel, -1);
    wxStaticLine *separator_line2 = new wxStaticLine(subpanel, -1);

    subsizer->Add(pc_text, 0, wxEXPAND);
    subsizer->Add(pc_display, 0, wxEXPAND);
    subsizer->Add(pc_go, 0, wxEXPAND);
    subsizer->Add(old_pc_text, 0, wxEXPAND | wxTOP, 3);
    subsizer->Add(old_pc, 0, wxEXPAND);
    subsizer->Add(separator_line, 0, wxEXPAND | wxALL, 5);
    subsizer->Add(go_address, 0, wxEXPAND);
    subsizer->Add(go_button, 0, wxEXPAND);
    subsizer->Add(separator_line2, 0, wxEXPAND | wxALL, 5);
    subsizer->Add(run, 0, wxEXPAND);
    subsizer->Add(pause, 0, wxEXPAND);
    subsizer->Add(step, 0, wxEXPAND);
    subpanel->SetSizer(subsizer);

    sizer->Add(code, 1, wxEXPAND);
    sizer->Add(scrollbar, 0, wxEXPAND);
    sizer->Add(subpanel, 0, wxEXPAND | wxALL, 5);
    SetSizer(sizer);

    go_address->Bind(wxEVT_COMMAND_TEXT_ENTER, &DisasmPanel::Goto, this);
    go_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Goto, this);
    pc_go->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::GotoPc, this);
    run->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Run, this);
    pause->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Pause, this);
    step->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Step, this);
    code->Bind(wxEVT_LEFT_DCLICK, &DisasmPanel::CodeDClick, this);

    scrollbar->Bind(wxEVT_SCROLL_THUMBRELEASE, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_TOP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_BOTTOM, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_LINEDOWN, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_LINEUP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_PAGEUP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_PAGEDOWN, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_THUMBTRACK, &DisasmPanel::Scrolled, this);

    Bind(wxEVT_MOUSEWHEEL, &DisasmPanel::MouseScroll, this);
}

DisasmPanel::~DisasmPanel()
{
    delete[] data;
    delete[] data_strings;
}

void DisasmPanel::Update(bool vi)
{
    if (!vi)
    {
        run->Enable(true);
        pause->Enable(false);
        char buf[16];
        int pc = parent->GetPc();
        if (pc)
        {
            code->SetPc(pc);
            sprintf(buf, "%08X", pc);
            pc_display->SetValue(buf);
            if (go_address->IsEmpty())
                go_address->SetValue(buf);

            sprintf(buf, "%08X", GetDebugState(M64P_DBG_PREVIOUS_PC));
            old_pc->SetValue(buf);
        }

        int lines = code->GetLines() - 2; // -1 because lines have the partially seen one included, another -1 because magic (bad?)
        uint32_t current_address = code->GetPos();
        int diff = (int)(pc - current_address) / 4;
        if (diff > lines || diff < 0)
        {
            if (diff > 2 * lines || diff < 0 - lines * 1) // Center if jumped more than one screen
                Goto(pc, 0);
            else if (diff < 0)
                Goto(pc, 1);
            else // lines < diff < 2 * lines
                Goto(current_address + (diff - lines) * 4, 1);
        }
    }
    else
    {
        code->SetPc(0x1);
        run->Enable(false);
        pause->Enable(true);
    }
    code->Render();
}

void DisasmPanel::BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update)
{
    if (last_update)
        code->Render();
}


void DisasmPanel::Run(wxCommandEvent &evt)
{
    code->Deselect();
    parent->Run();
}

void DisasmPanel::Pause(wxCommandEvent &evt)
{
    parent->Pause();
}

void DisasmPanel::Step(wxCommandEvent &evt)
{
    code->Deselect();
    parent->Step();
}

void DisasmPanel::Goto(wxCommandEvent &evt)
{
    Goto(strtoul(go_address->GetValue(), 0, 16), 1);
    code->Select(address, false);
}

void DisasmPanel::GotoPc(wxCommandEvent &evt)
{
    go_address->SetValue(pc_display->GetValue());
    Goto(strtoul(pc_display->GetValue(), 0, 16), 0);
}

void DisasmPanel::Goto(uint32_t new_address, int mode)
{
    if (mode == 0)
        new_address -= code->GetLines() * 2;
    new_address &= 0xfffffffc;
    if (new_address == address)
        return;
    address = new_address;

    code->Goto(address);
}

void DisasmPanel::MouseScroll(wxMouseEvent &evt)
{
    scroll_accum_delta += evt.GetWheelRotation();
    int scroll_delta = evt.GetWheelDelta();
    if (scroll_accum_delta >= scroll_delta)
    {
        Scroll(scroll_accum_delta / scroll_delta * -3); // Why these are reverse -.-
        scroll_accum_delta = scroll_accum_delta % scroll_delta;
    }
    else if (scroll_accum_delta <= 0 - scroll_delta)
    {
        Scroll(scroll_accum_delta / scroll_delta * -3);
        scroll_accum_delta = scroll_accum_delta % scroll_delta;
    }
}

void DisasmPanel::Scroll(int amt)
{
    if (amt > 0 && (uint32_t)(address + amt) < address)
        Goto(0xfffffffc, 2);
    else if (amt < 0 && (uint32_t)(address + amt) > address)
        Goto(0, 1);
    else
        Goto(address + amt * 4, 1);
}


void DisasmPanel::Scrolled(wxScrollEvent &evt)
{
    if (evt.GetEventType() != wxEVT_SCROLL_THUMBRELEASE)
    {
        int pos = evt.GetPosition();
        int line_change = (pos - scroll_thumbpos) * scroll_multiplier;
        Scroll(line_change);
        scroll_thumbpos = pos;
    }
    if (evt.GetEventType() != wxEVT_SCROLL_THUMBTRACK)
    {
        scrollbar->SetThumbPosition(scrollbar_thumb_defpos);
        scroll_thumbpos = scrollbar_thumb_defpos;
    }
}

const char **DisasmPanel::RequestData(int lines)
{
    uint32_t pos = code->GetPos();
    if (data_lines < lines)
    {
        delete[] data;
        delete[] data_strings;
        data = new const char *[lines];
        data_strings = new char[lines * 64]; // 64 bytes per instruction should be enough?
        data_lines = lines;
    }
    char *data_string_pos = data_strings;
    for (int i = 0; i < lines; i++)
    {
        char op[64], args[64];
        data[i] = data_string_pos;
        if (!MemIsValid(pos + i * 4))
        {
            strcpy(data_string_pos, "(Invalid)");
            data_string_pos += strlen("(Invalid)") + 1;
        }
        else
        {
            DecodeOpcode(pos + i * 4, op, args);

            // Core's decode function outputs some instructions, such as float manipulation, completely to the
            // "op" buffer and leaves args empty, adding two tabs between args and the opcode instead.
            // I don't understand the decode function well enough to edit it, so this just detects these tabs
            // and converts them to spaces.
            // I think decoder does that since those instructions share same opcode id, differing on smaller scale
            // than the other opcodes.

            char *tabs = strchr(op, '\t');
            if (tabs)
            {
                while (*tabs == '\t')
                {
                    *tabs = ' ';
                    tabs++;
                }
                data_string_pos += sprintf(data_string_pos, "%s", op) + 1;
            }
            else
                data_string_pos += sprintf(data_string_pos, "%s  %s", op, args) + 1;
        }
    }
    return data;
}

void DisasmPanel::ToggleBreakpoint(uint32_t bpt_address)
{
    Breakpoint *bpt = parent->FindBreakpoint(bpt_address);
    if (bpt)
    {
        // Let's touch only breakpoints that could have been added by this code
        if (bpt->GetLength() == 4 && bpt->GetAddress() == bpt_address && bpt->GetType() == BREAK_TYPE_EXECUTE)
            parent->DeleteBreakpoint(bpt);
        else
            return;
    }
    else
    {
        bpt = new Breakpoint(data[(bpt_address - code->GetPos()) / 4], bpt_address, 4, BREAK_TYPE_EXECUTE);
        parent->AddBreakpoint(bpt);
    }
}

void DisasmPanel::CodeDClick(wxMouseEvent &evt)
{
    int line = code->AddressHitTest(evt.GetPosition());
    if (line == -1)
        return;

    uint32_t address = code->GetPos() + line * 4;
    if (!MemIsValid(address))
        return;

    ToggleBreakpoint(address);
}

void DisasmPanel::Select(uint32_t address, bool add_to_selection)
{
    code->Select(address, add_to_selection);
}

/// ------------------------------------------------------------------------------

DisasmWindow::DisasmWindow(DisasmPanel *parent_, int id) : wxWindow(parent_, id, wxDefaultPosition, wxDefaultSize)
{
    parent = parent_;
    render_buffer = 0;
    lines = 0;
    address = 0;
    pc = 0x1; // Misaligned, so it won't be drawn
    drawn_pc = 0x1;
    select_start = 0x1;
    select_end = 0x1;

    Bind(wxEVT_SIZE, &DisasmWindow::Resize, this);
    Bind(wxEVT_PAINT, &DisasmWindow::Paint, this);
    Bind(wxEVT_LEFT_DOWN, &DisasmWindow::MouseClickEvt, this);
    Bind(wxEVT_CONTEXT_MENU, &DisasmWindow::RClickMenu, this);
}

DisasmWindow::~DisasmWindow()
{
    delete render_buffer;
}

int DisasmWindow::AddressHitTest(const wxPoint &pos)
{
    if (/*pos.x < address_start_x || */pos.x > address_start_x + address_width)
        return -1;
    if (pos.y < line_start_y - 2)
        return -1;
    return ((pos.y - line_start_y - 2) / line_height);
}

void DisasmWindow::Select(uint32_t address, bool add)
{
    if (!add || select_start == 0x1)
    {
        select_start = address;
        select_end = address;
    }
    else
    {
        if (address < select_start)
            select_start = address;
        else
            select_end = address;
    }
    if (!MemIsValid(select_start) && !MemIsValid(select_end))
    {
        select_start = 0x1;
        select_end = 0x1;
    }
    else
    {
        while (!MemIsValid(select_start))
            select_start += 4;
        while (!MemIsValid(select_end))
            select_end -= 4;
    }
    Render();
}

void DisasmWindow::Deselect()
{
    select_start = 0x1;
    select_end = 0x1;
    Render();
}

void DisasmWindow::MouseClick(const wxPoint &pos)
{
    if (pos.y < line_start_y - 2 || pos.x < opcode_start_x)
    {
        Deselect();
        return;
    }
    int row = (pos.y - line_start_y - 2) / line_height;
    Select(address + row * 4, wxGetKeyState(WXK_SHIFT));
}

void DisasmWindow::MouseClickEvt(wxMouseEvent &evt)
{
    evt.Skip();
    MouseClick(evt.GetPosition());
}

void DisasmWindow::RClickMenu(wxContextMenuEvent &evt)
{
    wxPoint pos = evt.GetPosition();
    if (pos != wxDefaultPosition)
    {
        pos = ScreenToClient(pos);
        MouseClick(pos);
        if (select_start == 0x1)
            return;
    }
    else
    {
        if (select_start == 0x1)
            return;

        pos = wxPoint(opcode_start_x, line_start_y + (select_start - address) / 4 * line_height);
    }

    wxMenu menu;
    wxMenuItem *copy_addr, *copy_ins, *set_break;

    copy_addr = new wxMenuItem(&menu, menu_id_copyaddr, _("Copy address"));
    copy_ins = new wxMenuItem(&menu, menu_id_copyins, _("Copy instruction"));
    set_break = new wxMenuItem(&menu, menu_id_break, _("Toggle breakpoint"));

    menu.Append(copy_addr);
    menu.Append(copy_ins);
    menu.Append(set_break);

    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &DisasmWindow::RClickEvent, this);
    PopupMenu(&menu, pos);
}

void DisasmWindow::RClickEvent(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case menu_id_copyaddr:
            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(wxString::Format("%08X", select_start)));
                wxTheClipboard->Close();
            }
        break;
        case menu_id_copyins:
            if (wxTheClipboard->Open())
            {
                wxTheClipboard->SetData(new wxTextDataObject(data[(select_start - address) / 4]));
                wxTheClipboard->Close();
            }
        break;
        case menu_id_break:
            parent->ToggleBreakpoint(select_start); // I don't think there is reason to care about multiselection..
        break;
    }
}


void DisasmWindow::Goto(uint32_t addr)
{
    address = addr;
    Render();
}

void DisasmWindow::Resize(wxSizeEvent &evt)
{
    wxSize size = evt.GetSize();

    int old_lines = lines;
    lines = (size.y - line_start_y) / line_height + 1;
    if (lines > old_lines || render_buffer->GetWidth() < size.x)
    {
        delete render_buffer;
        render_buffer = new wxBitmap(size);
    }
    Render();
}

void DisasmWindow::Paint(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    dc.DrawBitmap(*render_buffer, 0, 0);
}

void DisasmWindow::Render()
{
    if (!lines)
        return;

    data = parent->RequestData(lines);

    wxMemoryDC dc(*render_buffer);

    dc.SetFont(*g_main_font);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBackground(g_brush_bg);

    dc.Clear();

    dc.SetTextForeground(g_color_text_default);
    for (int i = 0; i < lines; i++)
    {
        uint32_t current_address = address + i * 4;
        bool current_line_selected = false;//, current_line_breakpoint = false;
        char buf[16];
        sprintf(buf, "%08X", current_address);
        Breakpoint *bpt = parent->GetParent()->FindBreakpoint(current_address);
        if (current_address == pc)
        {
            if (pc >= select_start && pc <= select_end)
                dc.SetBrush(g_brush_selected_pc);
            else
                dc.SetBrush(g_brush_pc);

            dc.DrawRectangle(0, line_start_y + i * line_height + 2, render_buffer->GetWidth(), line_height);
            dc.SetBrush(g_brush_bg);
        }
        else if (current_address >= select_start && current_address <= select_end)
        {
            dc.SetBrush(g_brush_selected);
            dc.DrawRectangle(0, line_start_y + i * line_height + 2, render_buffer->GetWidth(), line_height);
            dc.SetBrush(g_brush_bg);
            current_line_selected = true;
        }
        else
        {
            dc.DrawRectangle(0, line_start_y + i * line_height + 2, render_buffer->GetWidth(), line_height);
        }
        if (bpt)
        {
            if (bpt->GetType() & BREAK_TYPE_EXECUTE)
            {
                if (bpt->IsEnabled())
                    dc.SetBrush(g_brush_execute);
                else
                    dc.SetBrush(g_brush_disabled);
                dc.DrawRectangle(0, line_start_y + i * line_height + 2, address_width, line_height);
                dc.SetBrush(g_brush_bg);
                //current_line_breakpoint = true;
            }
        }
        if (current_line_selected) // I'm not sure what looks better, having always same text on break color or having it change if line is selected
            dc.SetTextForeground(g_color_text_selected); // Also not sure about the pc line color
        dc.DrawText(buf, address_start_x, line_start_y + i * line_height);
        dc.DrawText(data[i], opcode_start_x, line_start_y + i * line_height);
        if (current_line_selected)
            dc.SetTextForeground(g_color_text_default);
    }
    drawn_pc = pc;
    dc.SelectObject(wxNullBitmap);

    wxClientDC clientdc(this);
    clientdc.DrawBitmap(*render_buffer, 0, 0);
}
