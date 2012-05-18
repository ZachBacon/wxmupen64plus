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

#include "colors.h"
#include <wx/settings.h>
#include <wx/dcmemory.h>
#include <wx/frame.h>
#include <wx/textctrl.h>

wxBrush g_brush_execute;
wxBrush g_brush_read;
wxBrush g_brush_write;
wxBrush g_brush_disabled;
wxBrush g_brush_selected;
wxBrush g_brush_pc;
wxBrush g_brush_selected_pc;
wxBrush g_brush_bg;

wxColour g_color_text_selected;
wxColour g_color_text_default;

int g_number_width;
int g_bold_number_width;
int g_normal_height;
wxSize g_textctrl_default;

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

    wxColour selected = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    g_brush_selected.SetColour(selected);
    g_brush_pc.SetColour(0xfff070);
    g_brush_selected_pc.SetColour(wxColour((0x70 + selected.Red()) / 2, (0xff + selected.Green()) / 2, (0xff + selected.Blue()) / 2));

    g_brush_bg.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    g_color_text_selected = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    g_color_text_default = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    wxFrame *frame = new wxFrame(0, -1, "tmp");
    wxTextCtrl *ctrl = new wxTextCtrl(frame, -1);
    g_textctrl_default = ctrl->GetSize();
    frame->Destroy();
}
