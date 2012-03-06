#ifndef MEMORYPANEL_H
#define MEMORYPANEL_H

#include "debugpanel.h"
#include <wx/grid.h>
#include <stdint.h>

class wxDC;
class MemoryPanel;
class wxTextCtrl;
class wxButton;
class wxBitmap;

class MemoryWindow : public wxWindow
{
    public:
        MemoryWindow(MemoryPanel *parent, int id);
        ~MemoryWindow();

        void KeyDown(wxKeyEvent &evt);
        void MouseSelect(wxMouseEvent &evt);
        void PaintEvent(wxPaintEvent &evt);
        void Draw();

        void Resize(wxSizeEvent &evt);
        int GetRows() { return rows; }
        int GetCols() { return cols; }

        void Select(int pos);
        void Deselect(wxDC *dc = 0);
        void Goto(uint32_t offset);

        void Update();

        int GetSelected() { return selected; }

    private:
        wxBitmap *render_buffer;
        uint8_t currentvalue;
        bool editing;
        void Render(wxDC *dc);
        void RenderOffsets(wxDC *dc);
        void RenderValues(wxDC *dc);
        void DrawValue(wxDC *dc, int pos, const wxBrush *bg = 0);
        bool offsets_changed;

        wxPoint GetValuePosition(int index);

        int rows;
        int cols;
        int memory_size;
        int display_size; // memory_size rounded down to nearest cols
        uint32_t offset;
        int selected;
        uint8_t *data;
        MemoryPanel *parent;
};

class MemoryPanel : public DebugPanel
{
    public:
        MemoryPanel(DebuggerFrame *parent, int id = -1);
        virtual ~MemoryPanel();

        void Update(bool vi);

        void Select(int newpos);
        void Goto(wxCommandEvent &evt);

        uint8_t *RequestData(int size, int relative_offset = 0);
    private:
        void SetPosition(uint32_t offset, int size);

        MemoryWindow *memory;
        wxTextCtrl *offset_chooser;
        wxButton *goto_button;
        int data_size;
        bool data_changed;

        int current_row;
        int current_col;
        uint32_t current_position;

        void Select(int row, int col);
        uint8_t *data;
};

#endif // MEMORYPANEL_H
