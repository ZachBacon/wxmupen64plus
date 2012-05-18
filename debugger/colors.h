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

#ifndef D_COLORS_H
#define D_COLORS_H

#include <wx/colour.h>
#include <wx/brush.h>


extern wxBrush g_brush_execute;
extern wxBrush g_brush_read;
extern wxBrush g_brush_write;
extern wxBrush g_brush_disabled;
extern wxBrush g_brush_selected;
extern wxBrush g_brush_pc;
extern wxBrush g_brush_selected_pc;
extern wxBrush g_brush_bg;

extern wxColour g_color_text_selected;
extern wxColour g_color_text_default;

extern int g_number_width;
extern int g_bold_number_width;
extern int g_normal_height;
extern wxSize g_textctrl_default;

extern const wxFont *g_main_font;

void InitDrawingValues();

#endif // D_COLORS_H

