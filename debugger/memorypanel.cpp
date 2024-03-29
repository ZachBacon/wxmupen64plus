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

#include "memorypanel.h"

#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/menu.h>
#include "debugconfig.h"

#include "debuggerframe.h"
#include "colors.h"
#include "breakpoint.h"
#include "../mupen64plusplus/MupenAPI.h"
#ifdef DrawText
#undef DrawText // windows api ftw
#endif

// These control the drawing of MemoryWindow
#define offset_start_x 5
#define offset_start_y 5
#define data_start_x (offset_start_x + g_bold_number_width * 8 + 10)
#define data_start_y offset_start_y
#define data_inc_x (value_width + value_border_w) // Width reserved for 1 byte
#define data_add_inc_x (g_number_width) // Additional spacing after 4 bytes
#define data_inc_y (g_normal_height - 1) // Height reserved for 1 byte
#define data_clip_cols 4 // Should be multiple of 4
#define value_width (g_number_width * 2) // Actual width of 1 byte (about)
#define value_height (data_inc_y - value_border_h) // Actual height of 1 byte (about)
#define value_border_w 4 // These are total of the borders on both sides
#define value_border_h 2

enum
{
    read_break_1 = 1,
    read_break_2,
    read_break_4,
    write_break_1,
    write_break_2,
    write_break_4,
    remove_break
};

MemoryPanel::MemoryPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config) : DebugPanel(parent, id, type)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL), *subsizer = new wxBoxSizer(wxHORIZONTAL);
    data = 0;

    wxPanel *subpanel = new wxPanel(this);
    wxPanel *empty = new wxPanel(subpanel);
    offset_chooser = new wxTextCtrl(subpanel, -1, "0", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    offset_chooser->SetMaxLength(10);
    goto_button = new wxButton(subpanel, -1, _("Go"));
    subsizer->Add(empty, 1);
    subsizer->Add(offset_chooser, 0, wxEXPAND | wxALL, 2);
    subsizer->Add(goto_button, 0, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 2);
    subpanel->SetSizer(subsizer);

    memory = new MemoryWindow(this, -1);

    current_position = 0;
    uint32_t addr = strtoul(config.GetValue("Address", "80000000"), 0, 16);
    Goto(addr);
    offset_chooser->SetValue(wxString::Format("%08X", addr));

    sizer->Add(subpanel, 0, wxEXPAND);
    sizer->Add(memory, 1, wxEXPAND);
    SetSizer(sizer);

    current_row = 0;
    current_col = 0;
    data_changed = true;

    goto_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MemoryPanel::Goto, this);
    offset_chooser->Bind(wxEVT_COMMAND_TEXT_ENTER, &MemoryPanel::Goto, this);
}

MemoryPanel::~MemoryPanel()
{
    delete[] data;
}

void MemoryPanel::SaveConfig(DebugConfigOut &config, DebugConfigSection &section)
{
    section.num_values = 1;
    char buf[16];
    sprintf(buf, "%08X", memory->GetAddress());
    section.keys[0] = "Address";
    section.values[0] = buf;
    config.WriteSection(section);
}

void MemoryPanel::Update(bool vi)
{
    data_changed = true;
    memory->Update();
}

void MemoryPanel::SetPosition(uint32_t offset, int size)
{
    if (current_position != offset)
    {
        current_position = offset;
    }
    delete[] data;
    data = new uint8_t[size];
    for (int i = 0; i < size; i++)
    {
        data[i] = MemRead8(current_position + i);
    }
    data_changed = false;
    data_size = size;
}

void MemoryPanel::Goto(wxCommandEvent &evt)
{
    memory->Goto(strtoul(offset_chooser->GetValue(), 0, 16));
}

void MemoryPanel::Goto(uint32_t address)
{
    memory->Goto(address);
    offset_chooser->SetValue(wxString::Format("%08X", address));
}

uint8_t *MemoryPanel::RequestData(int size, int relative_offset)
{
    if (relative_offset != 0 || size > data_size || data_changed)
        SetPosition(current_position + relative_offset, size);
    return data;
}

void MemoryPanel::SetValue(int pos, int value)
{
    data[pos] = value;
    MemWrite8(current_position + pos, value);
}

void MemoryPanel::BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update)
{
    if (last_update)
        memory->Draw();
}

/// ------------------------------------------------------------------------------

MemoryWindow::MemoryWindow(MemoryPanel *parent_, int id) : wxWindow(parent_, id, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS)
{
    data = 0;
    parent = parent_;
    rows = 0;
    cols = 0;
    render_buffer = 0;
    address = 0;
    offsets_changed = true;
    selected = -1;
    Bind(wxEVT_PAINT, &MemoryWindow::PaintEvent, this);
    Bind(wxEVT_SIZE, &MemoryWindow::Resize, this);
    Bind(wxEVT_CHAR, &MemoryWindow::KeyDown, this);
    Bind(wxEVT_LEFT_DOWN, &MemoryWindow::MouseSelect, this);
    Bind(wxEVT_RIGHT_UP, &MemoryWindow::RClickMenu, this);
}

MemoryWindow::~MemoryWindow()
{
    delete render_buffer;
}

void MemoryWindow::Goto(uint32_t pos)
{
    if (address == pos)
        return;
    offsets_changed = true;
    data = parent->RequestData(cols * rows, pos - address);
    address = pos;
    if (!MemIsValid(address + selected))
        Deselect();
    Draw();
}

void MemoryWindow::MouseSelect(wxMouseEvent &evt)
{
    evt.Skip();
    wxPoint pos = evt.GetPosition(), last = GetValuePosition(display_size - 1);
    last.x += value_width + value_border_w / 2;
    last.y += value_height + value_border_h / 2;
    if (pos.x < data_start_x - value_border_w / 2 || pos.x > last.x || pos.y < data_start_y + value_border_h / 2 || pos.y > last.y)
    {
        Deselect();
        return;
    }

    pos.x = pos.x - data_start_x + value_border_w / 2;
    pos.y = pos.y - data_start_y - value_border_h / 2;
    int row = pos.y / data_inc_y;
    int x_incs = pos.x / (data_add_inc_x + data_inc_x * 4);
    int col = x_incs * 4;
    pos.x -= x_incs * (data_add_inc_x + data_inc_x * 4);
    col += pos.x / data_inc_x;
    last = GetValuePosition(row * cols + col);
    wxRect value_rect(last.x - value_border_w, last.y - value_border_h, last.x + value_border_w + value_width, last.y + value_border_h + value_height);
    if (value_rect.Contains(evt.GetPosition()))
       Select(row * cols + col);
    else
        Deselect();
}

void MemoryWindow::RClickMenu(wxMouseEvent &evt)
{
    MouseSelect(evt);
    if (selected != -1)
    {
        wxMenu menu;
        if (!parent->GetParent()->FindBreakpoint(address + selected))
        {
            wxMenuItem *rb1, *rb2, *rb4, *wb1, *wb2, *wb4;
            rb1 = new wxMenuItem(&menu, read_break_1, _("Read breakpoint (1 byte)"));
            menu.Append(rb1);
            if ((address + selected) % 2 == 0)
            {
                rb2 = new wxMenuItem(&menu, read_break_2, _("Read breakpoint (2 bytes)"));
                menu.Append(rb2);
            }
            if ((address + selected) % 4 == 0)
            {
                rb4 = new wxMenuItem(&menu, read_break_4, _("Read breakpoint (4 bytes)"));
                menu.Append(rb4);
            }
            wb1 = new wxMenuItem(&menu, write_break_1, _("Write breakpoint (1 byte)"));
            menu.Append(wb1);
            if ((address + selected) % 2 == 0)
            {
                wb2 = new wxMenuItem(&menu, write_break_2, _("Write breakpoint (2 bytes)"));
                menu.Append(wb2);
            }
            if ((address + selected) % 4 == 0)
            {
                wb4 = new wxMenuItem(&menu, write_break_4, _("Write breakpoint (4 bytes)"));
                menu.Append(wb4);
            }
        }
        else
        {
            wxMenuItem *rem;
            rem = new wxMenuItem(&menu, remove_break, _("Remove Breakpoint"));
            menu.Append(rem);
        }
        menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &MemoryWindow::RClickEvent, this);
        PopupMenu(&menu);
    }
}

void MemoryWindow::RClickEvent(wxCommandEvent &evt)
{
    int id = evt.GetId();
    DebuggerFrame *parent_frame = parent->GetParent();
    if (id == remove_break)
    {
        parent_frame->DeleteBreakpoint(parent_frame->FindBreakpoint(address + selected));
        Draw();
    }
    else
    {
        int selected_address = address + selected, length, type;
        switch (id)
        {
            case read_break_1:
                type = BREAK_TYPE_READ;
                length = 1;
            break;
            case read_break_2:
                type = BREAK_TYPE_READ;
                length = 2;
            break;
            case read_break_4:
                type = BREAK_TYPE_READ;
                length = 4;
            break;
            case write_break_1:
                type = BREAK_TYPE_WRITE;
                length = 1;
            break;
            case write_break_2:
                type = BREAK_TYPE_WRITE;
                length = 2;
            break;
            case write_break_4:
                type = BREAK_TYPE_WRITE;
                length = 4;
            break;
            default:
                type = 0;
                length = 0;
            break;
        }
        wxString name;
        if (type == BREAK_TYPE_READ)
            name.Printf("Read (%X)", length);
        else
            name.Printf("Write (%X)", length);

        Breakpoint *bpt = new Breakpoint(name, selected_address, length, type);
        if(!parent_frame->AddBreakpoint(bpt))
            parent->Print("Unable to add a new breakpoint");
    }
}

void MemoryWindow::Resize(wxSizeEvent &evt)
{
    wxSize size = evt.GetSize();
    int new_cols = (size.GetWidth() - data_start_x + data_add_inc_x) / (data_inc_x * data_clip_cols + data_add_inc_x * data_clip_cols / 4) * data_clip_cols;
    int new_rows = (size.GetHeight() - data_start_y) / data_inc_y;
    if (!new_cols)
        return;
    if (cols != new_cols || rows != new_rows)
    {
        cols = new_cols;
        rows = new_rows;
        display_size = rows * cols;
        data = parent->RequestData(cols * rows);
    }
    delete render_buffer;
    render_buffer = new wxBitmap(size);
    wxMemoryDC dc(*render_buffer);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    offsets_changed = true;

    Draw();
    return;
}

void MemoryWindow::Deselect()
{
    editing = false;
    if (selected != -1)
    {
        wxMemoryDC dc(*render_buffer);
        dc.SetFont(*g_main_font);
        DrawValue(&dc, selected, wxWHITE_BRUSH);
        wxClientDC clientdc(this);
        DrawValue(&clientdc, selected, wxWHITE_BRUSH);
        selected = -1;
    }
}

void MemoryWindow::Select(int pos)
{
    editing = false;
    if (!MemIsValid(address + pos))
        return;
    if (pos < 0)
    {
        int offset_change = (pos - cols) / cols * cols;
        if(!(pos % cols))
            offset_change += cols;
        if(address + offset_change < 0)
            offset_change = 0 - address;

        address += offset_change;
        data = parent->RequestData(display_size, offset_change);
        pos = (cols + pos % cols) % cols;
    }
    else if (pos >= display_size)
    {
        int offset_change = (pos - display_size + 1) / cols * cols + cols;
        if(!((pos + 1) % cols))
            offset_change -= cols;
        if(address + offset_change + display_size < address) // overflow
            offset_change = 0 - address - display_size;
        address += offset_change;
        data = parent->RequestData(display_size, offset_change);
        pos = pos % cols + cols * (rows - 1);
    }

    selected = pos;
    Draw();
}

void MemoryWindow::Input(int value)
{
    if (selected == -1)
        return;
    if (editing)
    {
        editing = false;
        parent->SetValue(selected, (new_value << 4) | value);
        Select(selected + 1);
    }
    else
    {
        editing = true;
        new_value = value;
        Draw();
    }
}

void MemoryWindow::KeyDown(wxKeyEvent &evt)
{
    if (selected == -1)
        return;

    int code = evt.GetKeyCode();
    if (code >= '0' && code <= '9')
        Input(code - '0');
    else if (code >= 'a' && code <= 'f')
        Input(code - 'a' + 0xa);
    else if (code >= 'A' && code <= 'F')
        Input(code - 'A' + 0xa);
    else
    {
        switch (code)
        {
            case WXK_LEFT:
                Select(selected - 1);
            break;
    //        case WXK_RETURN: // nah
    //        case WXK_NUMPAD_ENTER:
            case WXK_RIGHT:
                Select(selected + 1);
            break;
            case WXK_UP:
                Select(selected - cols);
            break;
            case WXK_DOWN:
                Select(selected + cols);
            break;

            default:
                evt.Skip();
            break;
        }
    }
}

void MemoryWindow::PaintEvent(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    wxRegion damaged = GetUpdateRegion();
    if (damaged.Contains(0, 0, data_start_x + (cols + 1) * data_inc_x, data_start_y + (rows + 1) * data_inc_y) == wxOutRegion)
    {
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        return;
    }
    dc.DrawBitmap(*render_buffer, 0, 0);
}

wxPoint MemoryWindow::GetValuePosition(int index)
{
    wxPoint ret;
    int col = (index % cols);
    ret.x = data_start_x + data_inc_x * col + col / 4 * data_add_inc_x;
    ret.y = data_start_y + data_inc_y * (index / cols);
    return ret;
}

void MemoryWindow::DrawValue(wxDC *dc, int pos, const wxBrush *bg, const char *value)
{
    wxPoint point = GetValuePosition(pos);
    if (bg)
    {
        dc->SetPen(*wxTRANSPARENT_PEN);
        dc->SetBrush(*bg);
        dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
    }
    else if (Breakpoint *bpt = parent->GetParent()->FindBreakpoint(address + pos))
    {
        dc->SetPen(*wxTRANSPARENT_PEN);
        int type = bpt->GetType(), count = 0;
        if (!bpt->IsEnabled())
        {
            dc->SetBrush(g_brush_disabled);
            dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
        }
        else if (type == BREAK_TYPE_ALL) // A special case to keep the color order always as exec-read-write
        {
            dc->SetBrush(g_brush_execute);
            dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
            dc->SetBrush(g_brush_write);
            dc->DrawRectangle(point.x + value_width / 2, point.y + value_border_h / 2, (value_width + value_border_w) / 2, value_height + value_border_h);
            dc->SetBrush(g_brush_read);
            dc->DrawRectangle(point.x + (value_width + value_border_w) / 4, point.y + value_border_h / 2, // Yeah, / 4. It just works better..
                               (value_width + value_border_w) / 3, value_height + value_border_h);
        }
        else
        {
            if (type & BREAK_TYPE_EXECUTE)
            {
                dc->SetBrush(g_brush_execute);
                dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
                count++;
            }
            if (type & BREAK_TYPE_READ)
            {
                dc->SetBrush(g_brush_read);
                if (!count)
                    dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
                else
                    dc->DrawRectangle(point.x + value_width / 2, point.y + value_border_h / 2,
                                      (value_width + value_border_w) / 2, value_height + value_border_h);
                count++;
            }
            if (type & BREAK_TYPE_WRITE)
            {
                dc->SetBrush(g_brush_write);
                if (!count)
                    dc->DrawRectangle(point.x - value_border_w / 2, point.y + value_border_h / 2, value_width + value_border_w, value_height + value_border_h);
                else
                    dc->DrawRectangle(point.x + value_width / 2, point.y + value_border_h / 2,
                                      (value_width + value_border_w) / 2, value_height + value_border_h);
            }
        }
    }
    char buf[4];
    if (!MemIsValid(address + pos))
        value = "--";
    else if (!value)
    {
        sprintf(buf, "%02X", data[pos]);
        value = buf;
    }
    dc->DrawText(value, point);
}

void MemoryWindow::RenderValues(wxDC *dc)
{
    dc->SetPen(*wxTRANSPARENT_PEN);
    dc->SetBrush(*wxWHITE_BRUSH);
    wxPoint last = GetValuePosition(display_size - 1);
    dc->DrawRectangle(data_start_x - value_border_w / 2, data_start_y - value_border_h / 2, last.x + value_border_w * 2 + value_width, last.y + value_border_h + value_height);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            DrawValue(dc, i * cols + j);
        }
    }
}

void MemoryWindow::RenderOffsets(wxDC *dc)
{
    dc->SetPen(*wxTRANSPARENT_PEN);
    dc->SetBrush(*wxWHITE_BRUSH);
    dc->DrawRectangle(offset_start_x, offset_start_y, data_start_x - offset_start_x, data_inc_y * rows);
    for (int i = 0; i < rows; i++)
    {
        char buf[16];
        sprintf(buf, "%08X", address + i * cols);
        dc->DrawText(buf, offset_start_x, offset_start_y + data_inc_y * i);
    }
    offsets_changed = false;
}

void MemoryWindow::Render(wxDC *dc)
{
    dc->SetTextForeground(g_color_text_default);
    if (offsets_changed)
    {
        wxFont orig = dc->GetFont();
        wxFont bold = orig.Bold();
        dc->SetFont(bold);
        RenderOffsets(dc);
        dc->SetFont(orig);
    }
    RenderValues(dc);

    if (selected != -1)
    {
        dc->SetTextForeground(g_color_text_selected);
        if (editing)
        {
            char buf[4];
            sprintf(buf, "%X_", new_value);
            DrawValue(dc, selected, &g_brush_selected, buf);
        }
        else
            DrawValue(dc, selected, &g_brush_selected);
    }
}

void MemoryWindow::Draw()
{
    if (!render_buffer)
        return;

    // Destroy memorydc before using bitmap :p
    {
        wxMemoryDC dc(*render_buffer);
        dc.SetFont(*g_main_font);
        Render(&dc);
    }
    wxClientDC client_dc(this);
    client_dc.DrawBitmap(*render_buffer, 0, 0);
}

void MemoryWindow::Update()
{
    if (!cols)
        return;
    data = parent->RequestData(rows * cols);
    Draw();
}
