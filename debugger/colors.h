#ifndef D_COLORS_H
#define D_COLORS_H

#include <wx/colour.h>
#include <wx/brush.h>


extern wxBrush g_brush_execute;
extern wxBrush g_brush_read;
extern wxBrush g_brush_write;
extern wxBrush g_brush_disabled;
extern wxBrush g_brush_selected;

extern wxColour g_color_text_selected;
extern wxColour g_color_text_default;

extern int g_number_width;
extern int g_bold_number_width;
extern int g_normal_height;

extern const wxFont *g_main_font;

void InitDrawingValues();

#endif // D_COLORS_H

