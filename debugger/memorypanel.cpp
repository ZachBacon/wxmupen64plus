#include "memorypanel.h"

#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/textctrl.h>
#include <wx/button.h>

#include "colors.h"
#include "breakpoint.h"
#include "../mupen64plusplus/MupenAPI.h"
#ifdef DrawText
#undef DrawText // windows api ftw
#endif

// These control the drawing of MemoryWindow
#define offset_start_x 5
#define offset_start_y 5
#define data_start_x 70
#define data_start_y offset_start_y
#define data_inc_x 16 // Width reserved for 1 byte
#define data_add_inc_x 6 // Additional spacing after 4 bytes
#define data_inc_y 12 // Height reserved for 1 byte
#define data_clip_cols 4 // Should be multiple of 4
#define value_width 12 // Actual width of 1 byte (about)
#define value_height 10 // Actual height of 1 byte (about)
#define value_border_w 4
#define value_border_h 2

MemoryPanel::MemoryPanel(DebuggerFrame *parent, int id) : DebugPanel(parent, id)
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

    sizer->Add(subpanel, 0, wxEXPAND);
    sizer->Add(memory, 1, wxEXPAND);
    SetSizer(sizer);

    current_row = 0;
    current_col = 0;
    data_changed = true;

    goto_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MemoryPanel::Goto, this);
    offset_chooser->Bind(wxEVT_COMMAND_TEXT_ENTER, &MemoryPanel::Goto, this);
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
    uint32_t offset = strtoul(offset_chooser->GetValue(), 0, 16);
    memory->Goto(offset);
}

uint8_t *MemoryPanel::RequestData(int size, int relative_offset)
{
    if (relative_offset != 0 || size > data_size || data_changed)
        SetPosition(current_position + relative_offset, size);
    return data;
}

MemoryPanel::~MemoryPanel()
{
}

void MemoryPanel::SetValue(int pos, int value)
{
    data[pos] = value;
    MemWrite8(current_position + pos, value);
}

/// ------------------------------------------------------------------------------

MemoryWindow::MemoryWindow(MemoryPanel *parent_, int id) : wxWindow(parent_, id, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS)
{
    data = 0;
    parent = parent_;
    rows = 0;
    cols = 0;
    render_buffer = 0;
    offset = 0;
    offsets_changed = true;
    selected = -1;
    Bind(wxEVT_PAINT, &MemoryWindow::PaintEvent, this);
    Bind(wxEVT_SIZE, &MemoryWindow::Resize, this);
    Bind(wxEVT_CHAR, &MemoryWindow::KeyDown, this);
    Bind(wxEVT_LEFT_DOWN, &MemoryWindow::MouseSelect, this);
}

MemoryWindow::~MemoryWindow()
{
    delete render_buffer;
}

void MemoryWindow::Goto(uint32_t pos)
{
    if (offset == pos)
        return;
    offsets_changed = true;
    data = parent->RequestData(cols * rows, pos - offset);
    offset = pos;
    if (!MemIsValid(offset + selected))
        Deselect();
    Draw();
}

void MemoryWindow::MouseSelect(wxMouseEvent &evt)
{
    evt.Skip();
    wxPoint pos = evt.GetPosition(), last = GetValuePosition(display_size - 1);
    last.x += value_border_w + value_width;
    last.y += value_border_h + value_height;
    if (pos.x < data_start_x || pos.x > last.x || pos.y < data_start_y || pos.y > last.y)
    {
        Deselect();
        return;
    }

    pos.x -= data_start_x - value_border_w / 2;
    pos.y -= data_start_y - value_border_h / 2;
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

void MemoryWindow::Resize(wxSizeEvent &evt)
{
    wxSize size = evt.GetSize();
    int new_cols = (size.GetWidth() - data_start_x) / (data_inc_x * data_clip_cols + data_add_inc_x * data_clip_cols / 4) * data_clip_cols;
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
        dc.SetFont(*wxNORMAL_FONT);
        DrawValue(&dc, selected, wxWHITE_BRUSH);
        wxClientDC clientdc(this);
        DrawValue(&clientdc, selected, wxWHITE_BRUSH);
        selected = -1;
    }
}

void MemoryWindow::Select(int pos)
{
    editing = false;
    if (!MemIsValid(offset + pos))
        return;
    if (pos < 0)
    {
        int offset_change = (pos - cols) / cols * cols;
        if(!(pos % cols))
            offset_change += cols;
        if(offset + offset_change < 0)
            offset_change = 0 - offset;

        offset += offset_change;
        data = parent->RequestData(display_size, offset_change);
        pos = (cols + pos % cols) % cols;
    }
    else if (pos >= display_size)
    {
        int offset_change = (pos - display_size + 1) / cols * cols + cols;
        if(!((pos + 1) % cols))
            offset_change -= cols;
        if(offset + offset_change + display_size < offset) // overflow
            offset_change = 0 - offset - display_size;
        offset += offset_change;
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
    else if (Breakpoint *bpt = Breakpoint::Find(offset + pos))
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
    if (!MemIsValid(offset + pos))
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
        sprintf(buf, "%08X", offset + i * cols);
        dc->DrawText(buf, offset_start_x, offset_start_y + data_inc_y * i);
    }
    offsets_changed = false;
}

void MemoryWindow::Render(wxDC *dc)
{
    if (!data)
        return;

    dc->SetTextForeground(*wxBLACK);
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
        dc->SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
        wxBrush select_bg(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        if (editing)
        {
            char buf[4];
            sprintf(buf, "%X_", new_value);
            DrawValue(dc, selected, &select_bg, buf);
        }
        else
            DrawValue(dc, selected, &select_bg);
    }
}

void MemoryWindow::Draw()
{
    // Destruct memorydc before using bitmap :p
    {
        wxMemoryDC dc(*render_buffer);
        dc.SetFont(*wxNORMAL_FONT);
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
