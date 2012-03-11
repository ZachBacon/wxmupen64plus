#ifndef REGISTERPANEL_H
#define REGISTERPANEL_H

#include "debugpanel.h"
#include <vector>
#include <wx/scrolwin.h>


class wxNotebook;
class wxTextCtrl;

// After four hours of trying to get both sizer and scrolling stuff work with
// the layout changing when it can fit another column, I just scrapped it all..
// Might be less platform-independent :(

enum RegisterType
{
    REGISTER_INT64 = 1,
    REGISTER_INT32,
    REGISTER_FLOAT,
};

enum RegisterGroup
{
    REGISTER_GPR = 1,
    REGISTER_COP0,
    REGISTER_COP1
};

class RegChangeEvent;
extern const wxEventTypeTag<RegChangeEvent> REGVAL_CHANGE_EVT;
class RegChangeEvent : public wxEvent
{
    public:
        RegChangeEvent(int id) : wxEvent(id, REGVAL_CHANGE_EVT) {}
        virtual wxEvent *Clone() const { return new RegChangeEvent(*this); }
        wxAny value;
};

wxDECLARE_EVENT(REGVAL_CHANGE_EVT, RegChangeEvent);

class SingleRegister : public wxPanel
{
    public:
        SingleRegister(wxWindow *parent, int id, const char *name, RegisterType type, wxPoint &pos, int reg_name_len);
        ~SingleRegister();
        uint64_t GetInt();
        double GetFloat();

        void SetInt(uint64_t value);
        void SetFloat(double value);
        void ResetValue();
        void ValueChanged(wxCommandEvent &evt);
        void ValueChanged();

        void RClickMenu(wxMouseEvent &evt);
        void RClickEvent(wxCommandEvent &evt);

    private:
        bool setting_value;
        wxAny original_value;
        wxTextCtrl *value;
        wxTextCtrl *value2;
        RegisterType type;
};

class RegisterTab : public wxScrolledWindow
{
    public:
        RegisterTab(wxWindow *parent, int id, RegisterGroup type);
        ~RegisterTab();

        void Size(wxSizeEvent &evt);

        void Append(const char *name, RegisterType type, int id = -1);
        void ValueChanged(RegChangeEvent &evt);

        void SetInt(int index, uint64_t value);
        void SetFloat(int index, double value);

        void Update();

    private:
        void InitRegisters(RegisterGroup type_);

        wxPoint CalcItemPos(int index);
        union
        {
            uint64_t *i64;
            uint32_t *i32;
            double *f;
            void *v;
        } raw_registers;

        bool show_reserved;
        void Reorder();
        int cols;
        int rows;
        int reg_name_len;
        int reg_basewidth;
        RegisterGroup type;
        std::vector<SingleRegister *> registers;
};

class RegisterPanel : public DebugPanel
{
    public:
        RegisterPanel(DebuggerFrame *parent, int id = -1);
        virtual ~RegisterPanel();

        void Update(bool vi);

    private:

        RegisterTab *gpr_tab;
        RegisterTab *cop0_tab;
        RegisterTab *cop1_tab;
        wxNotebook *notebook;
};

#endif // REGISTERPANEL_H

