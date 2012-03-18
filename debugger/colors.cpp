#include "colors.h"
#include <wx/settings.h>

// These are BGR and not RGB btw
wxBrush g_brush_execute((wxColour)0x0000ff);
wxBrush g_brush_read((wxColour)0x00c0c0);
wxBrush g_brush_write((wxColour)0x008000);
wxBrush g_brush_disabled((wxColour)0x808080);
wxBrush g_brush_selected(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

wxColour g_color_text_selected(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
wxColour g_color_text_default(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
