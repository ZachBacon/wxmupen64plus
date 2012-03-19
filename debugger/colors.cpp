#include "colors.h"
#include <wx/settings.h>
#include <wx/dcmemory.h>

// These are BGR and not RGB btw
wxBrush g_brush_execute((wxColour)0x0000ff);
wxBrush g_brush_read((wxColour)0x00c0c0);
wxBrush g_brush_write((wxColour)0x008000);
wxBrush g_brush_disabled((wxColour)0x808080);
wxBrush g_brush_selected(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

wxColour g_color_text_selected(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
wxColour g_color_text_default(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

int g_number_width;
int g_bold_number_width;
int g_normal_height;

const wxFont *g_main_font = wxNORMAL_FONT;

void InitDrawingValues()
{
    wxMemoryDC dc;
    dc.SetFont(*g_main_font);
    g_number_width = dc.GetTextExtent("0").x;
    g_normal_height = dc.GetCharHeight();
    dc.SetFont(dc.GetFont().Bold());
    g_bold_number_width = dc.GetTextExtent("0").x;
}
