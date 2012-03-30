#include "colors.h"
#include <wx/settings.h>
#include <wx/dcmemory.h>

wxBrush g_brush_execute;
wxBrush g_brush_read;
wxBrush g_brush_write;
wxBrush g_brush_disabled;
wxBrush g_brush_selected;
wxBrush g_brush_pc;
wxBrush g_brush_bg;

wxColour g_color_text_selected;
wxColour g_color_text_default;

int g_number_width;
int g_bold_number_width;
int g_normal_height;

const wxFont *g_main_font;

void InitDrawingValues()
{
    g_main_font = wxNORMAL_FONT;

    wxMemoryDC dc;
    dc.SetFont(*g_main_font);
    g_number_width = dc.GetTextExtent("0").x;
    g_normal_height = dc.GetCharHeight();
    dc.SetFont(dc.GetFont().Bold());
    g_bold_number_width = dc.GetTextExtent("0").x;

    // These are BGR and not RGB btw
    g_brush_execute.SetColour((wxColour)0x0000ff);
    g_brush_read.SetColour((wxColour)0x00c0c0);
    g_brush_write.SetColour((wxColour)0x008000);
    g_brush_disabled.SetColour(0x808080);
    g_brush_selected.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    g_brush_pc.SetColour(0xfff070);
    g_brush_bg.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    g_color_text_selected = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    g_color_text_default = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
}

