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

#ifndef DISASMPANEL_H
#define DISASMPANEL_H

#include "debugpanel.h"

class wxDC;
class DisasmWindow;
class wxButton;
class wxTextCtrl;
class wxScrollBar;

class DisasmPanel : public DebugPanel
{
    public:
        DisasmPanel(DebuggerFrame *parent, int id, int type, DebugConfigSection &config);
        virtual ~DisasmPanel();

        void Update(bool vi);
        void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update);

        const char **RequestData(int lines);

        void Scrolled(wxScrollEvent &evt);
        void MouseScroll(wxMouseEvent &evt);
        void Scroll(int amt);

        void ToggleBreakpoint(uint32_t bpt_address);

        void CodeDClick(wxMouseEvent &evt);
        void Run(wxCommandEvent &evt);
        void Pause(wxCommandEvent &evt);
        void Step(wxCommandEvent &evt);
        void Goto(wxCommandEvent &evt);
        void GotoPc(wxCommandEvent &evt);
        void Goto(uint32_t new_address, int mode); // 0 = center, 1 = up
        void Select(uint32_t address, bool add_to_selection);

    private:
        DisasmWindow *code;

        const char **data;
        char *data_strings;
        short data_lines;

        wxScrollBar *scrollbar;
        int scroll_thumbpos;
        int scroll_accum_delta;

        wxTextCtrl *go_address;
        wxButton *go_button;
        wxButton *pc_go;
        wxButton *run;
        wxButton *pause;
        wxButton *step;
        wxTextCtrl *pc_display;
        wxTextCtrl *old_pc;

        uint32_t address;
};


class DisasmWindow : public wxWindow
{
    public:
        DisasmWindow(DisasmPanel *parent, int id);
        ~DisasmWindow();

        void Resize(wxSizeEvent &evt);
        void Render();
        void Paint(wxPaintEvent &evt);
        void MouseClickEvt(wxMouseEvent &evt);
        void MouseClick(const wxPoint &pos);

        void Select(uint32_t address, bool add_to_selection);
        void Deselect();
        void Goto(uint32_t addr);
        void SetPc(uint32_t pc_) { pc = pc_; }

        int GetLines() { return lines; }
        uint32_t GetPos() { return address; }

        int AddressHitTest(const wxPoint &pos);

        void RClickMenu(wxContextMenuEvent &evt);
        void RClickEvent(wxCommandEvent &evt);

    private:
        uint32_t address;
        uint32_t pc;
        uint32_t drawn_pc;
        uint32_t select_start;
        uint32_t select_end;
        int lines;
        const char **data;

        wxBitmap *render_buffer;
        DisasmPanel *parent;
};

#endif // DISASMPANEL_H

