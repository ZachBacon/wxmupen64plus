#include "disasmpanel.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/scrolbar.h>
#include <wx/stattext.h>
#include <wx/statline.h>

#include "debuggerframe.h"
#include "../mupen64plusplus/MupenAPI.h"

#ifdef DrawText
#undef DrawText // windows api ftw
#endif

#define line_start_y 1
#define address_start_x 5
#define opcode_start_x address_start_x + 70
#define comment_start_x opcode_start_x + 500
#define line_height 10

#define scrollbar_size 0x80c // Maybe these could be member vars, changing when resized?
#define scrollbar_thumb 10 // Though now they have nice 0x4, 0x100 and (almost) 0x1000 scroll steps
#define scrollbar_thumb_defpos (scrollbar_size - scrollbar_thumb) / 2
#define scroll_multiplier 1

DisasmPanel::DisasmPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL), *subsizer = new wxBoxSizer(wxVERTICAL);
    address = 0;
    data_lines = 0;
    data = 0;
    data_strings = 0;

    code = new DisasmWindow(this, -1);
    scrollbar = new wxScrollBar(this, -1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    scrollbar->SetScrollbar(scrollbar_thumb_defpos, scrollbar_thumb, scrollbar_size, 0x40);
    scroll_thumbpos = scrollbar_thumb_defpos;

    wxPanel *subpanel = new wxPanel(this, -1);
    go_address = new wxTextCtrl(subpanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    go_address->SetMaxLength(10);
    go_address->SetMinSize(wxSize(70, -1));
    go_button = new wxButton(subpanel, -1, _("Go"));
    wxStaticText *pc_text = new wxStaticText(subpanel, -1, _("PC"));
    pc_display = new wxTextCtrl(subpanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    pc_display->SetMinSize(wxSize(70, -1));
    pc_go = new wxButton(subpanel, -1, _("Go to PC"));
    step = new wxButton(subpanel, -1, _("Step"));
    wxStaticLine *separator_line = new wxStaticLine(subpanel, -1);
    wxStaticLine *separator_line2 = new wxStaticLine(subpanel, -1);

    subsizer->Add(pc_text, 0, wxEXPAND);
    subsizer->Add(pc_display, 0, wxEXPAND);
    subsizer->Add(pc_go, 0, wxEXPAND);
    subsizer->Add(separator_line, 0, wxEXPAND | wxALL, 5);
    subsizer->Add(go_address, 0, wxEXPAND);
    subsizer->Add(go_button, 0, wxEXPAND);
    subsizer->Add(separator_line2, 0, wxEXPAND | wxALL, 5);
    subsizer->Add(step, 0, wxEXPAND);
    subpanel->SetSizer(subsizer);

    sizer->Add(code, 1, wxEXPAND);
    sizer->Add(scrollbar, 0, wxEXPAND);
    sizer->Add(subpanel, 0, wxEXPAND | wxALL, 5);
    SetSizer(sizer);

    go_address->Bind(wxEVT_COMMAND_TEXT_ENTER, &DisasmPanel::Goto, this);
    go_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Goto, this);
    pc_go->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::GotoPc, this);
    step->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DisasmPanel::Step, this);

    scrollbar->Bind(wxEVT_SCROLL_THUMBRELEASE, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_TOP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_BOTTOM, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_LINEDOWN, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_LINEUP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_PAGEUP, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_PAGEDOWN, &DisasmPanel::Scrolled, this);
    scrollbar->Bind(wxEVT_SCROLL_THUMBTRACK, &DisasmPanel::Scrolled, this);
}

DisasmPanel::~DisasmPanel()
{
}

void DisasmPanel::Update(bool vi)
{
    if (!vi)
    {
        char buf[16];
        int pc = parent->GetPc();
        code->SetPc(pc);
        sprintf(buf, "%X", pc);
        pc_display->SetValue(buf);
        go_address->SetValue(buf);

//        int diff = (int)(code->GetPos() - address) / 4;
//        if (abs(diff) > lines / 2)
//
//        Goto(pc);
    }
    else
        code->SetPc(0x1);
    code->Render(true);
}

void DisasmPanel::Step(wxCommandEvent &evt)
{
    parent->Step();
}

void DisasmPanel::Goto(wxCommandEvent &evt)
{
    Goto(strtoul(go_address->GetValue(), 0, 16));
}

void DisasmPanel::GotoPc(wxCommandEvent &evt)
{
    go_address->SetValue(pc_display->GetValue());
    Goto(strtoul(pc_display->GetValue(), 0, 16));
}

void DisasmPanel::Goto(uint32_t new_address)
{
    int lines = code->GetLines();
    new_address -= lines * 2;
    new_address &= 0xfffffffc;
    if (new_address == address)
        return;
    address = new_address;

    code->Goto(address);
}

void DisasmPanel::Scrolled(wxScrollEvent &evt)
{
    if (evt.GetEventType() != wxEVT_SCROLL_THUMBRELEASE)
    {
        int pos = evt.GetPosition();
        int line_change = (pos - scroll_thumbpos) * scroll_multiplier;
        scroll_thumbpos = pos;
        if (line_change > 0 && (uint32_t)(address + line_change) < address)
            Goto(0 - code->GetLines() * 2);
        else if (line_change < 0 && (uint32_t)(address + line_change) > address)
            Goto(0 + code->GetLines() * 2);
        else
            Goto(address + line_change * 4 + code->GetLines() * 2);
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
            data_string_pos += sprintf(data_string_pos, "%s  %s", op, args) + 1;
        }
    }
    return data;
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

    Bind(wxEVT_SIZE, &DisasmWindow::Resize, this);
    Bind(wxEVT_PAINT, &DisasmWindow::Paint, this);
}

DisasmWindow::~DisasmWindow()
{
    delete render_buffer;
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
    if (lines < old_lines)
        return;
    else if (lines > old_lines)
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

void DisasmWindow::Render(bool same_address)
{
    data = parent->RequestData(lines);

    wxMemoryDC dc(*render_buffer);
    wxColour bg_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    dc.SetFont(*wxNORMAL_FONT);
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxBrush bg(bg_colour);
    dc.SetBackground(bg);

    if (!same_address)
        dc.Clear();
    else
        dc.DrawRectangle(opcode_start_x, line_start_y, comment_start_x - opcode_start_x, line_height * lines);

    for (int i = 0; i < lines; i++)
    {
        char buf[16];
        if (address + i * 4 == pc)
        {
            sprintf(buf, "%X", address + i * 4);
            dc.SetBrush(*wxCYAN_BRUSH);
            dc.DrawRectangle(0, line_start_y + i * line_height + 2, render_buffer->GetWidth(), line_height);
            dc.DrawText(buf, address_start_x, line_start_y + i * line_height);
            dc.DrawText(data[i], opcode_start_x, line_start_y + i * line_height);
            dc.SetBrush(bg);
        }
        else
        {
            if (!same_address || address + i * 4 == drawn_pc)
            {
                sprintf(buf, "%X", address + i * 4);
                dc.DrawRectangle(0, line_start_y + i * line_height + 2, render_buffer->GetWidth(), line_height);
                dc.DrawText(buf, address_start_x, line_start_y + i * line_height);
            }
            dc.DrawText(data[i], opcode_start_x, line_start_y + i * line_height);
        }
    }
    drawn_pc = pc;
    dc.SelectObject(wxNullBitmap);

    wxClientDC clientdc(this);
    clientdc.DrawBitmap(*render_buffer, 0, 0);
}
