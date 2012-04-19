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

        void SaveConfig(DebugConfigSection &config);
        void Update(bool vi);
        void BreakpointUpdate(Breakpoint *bpt, BreakUpdateCause cause, bool last_update);

        const char **RequestData(int lines);

        void Scrolled(wxScrollEvent &evt);
        void MouseScroll(wxMouseEvent &evt);
        void Scroll(int amt);

        void CodeDClick(wxMouseEvent &evt);
        void Run(wxCommandEvent &evt);
        void Pause(wxCommandEvent &evt);
        void Step(wxCommandEvent &evt);
        void Goto(wxCommandEvent &evt);
        void GotoPc(wxCommandEvent &evt);
        void Goto(uint32_t address, int mode); // 0 = center, 1 = up

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
        void MouseClick(wxMouseEvent &evt);

        void Select(uint32_t address, bool add_to_selection);
        void Deselect();
        void Goto(uint32_t addr);
        void SetPc(uint32_t pc_) { pc = pc_; }

        int GetLines() { return lines; }
        uint32_t GetPos() { return address; }

        int AddressHitTest(const wxPoint &pos);

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

