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
        DisasmPanel(DebuggerFrame *parent, int id = -1);
        virtual ~DisasmPanel();

        void Update(bool vi);

        const char **RequestData(int lines);

        void Scrolled(wxScrollEvent &evt);
        void Goto(wxCommandEvent &evt);
        void Goto(uint32_t address);

    private:
        DisasmWindow *code;

        const char **data;
        char *data_strings;
        short data_lines;

        wxScrollBar *scrollbar;
        int scroll_thumbpos;
        wxTextCtrl *go_address;
        wxButton *go_button;

        uint32_t address;
};


class DisasmWindow : public wxWindow
{
    public:
        DisasmWindow(DisasmPanel *parent, int id);
        ~DisasmWindow();

        void Resize(wxSizeEvent &evt);
        void Render(bool same_address = false);
        void Paint(wxPaintEvent &evt);

        void Goto(uint32_t addr);

        int GetLines() { return lines; }

    private:
        uint32_t address;
        int lines;
        const char **data;

        wxBitmap *render_buffer;
        DisasmPanel *parent;
};

#endif // DISASMPANEL_H

