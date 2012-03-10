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
    REGISTER_FLOAT
};

class SingleRegister : public wxPanel
{
    public:
        SingleRegister(wxWindow *parent, int id, const char *name, RegisterType type, wxPoint &pos, int reg_name_len);
        ~SingleRegister();
        uint64_t GetInt();
        double GetFloat();

        void SetInt(uint64_t value);
        void SetFloat(double value);

    private:
        wxTextCtrl *value;
        wxTextCtrl *value2;
        RegisterType type;
};

class RegisterTab : public wxScrolledWindow
{
    public:
        RegisterTab(wxWindow *parent, int id, int reg_name_len, int reg_basewidth);
        ~RegisterTab();

        void Size(wxSizeEvent &evt);

        void Append(const char *name, RegisterType type);

        void SetInt(int index, uint64_t value);
        void SetFloat(int index, double value);

    private:
        wxPoint CalcItemPos(int index);
        void Reorder();
        int cols;
        int rows;
        int reg_name_len;
        int reg_basewidth;
        std::vector<SingleRegister *> registers;
};

class RegisterPanel : public DebugPanel
{
    public:
        RegisterPanel(DebuggerFrame *parent, int id = -1);
        virtual ~RegisterPanel();

        void Update(bool vi);

    private:
        void UpdateGpr();
        void UpdateCop0();
        void UpdateCop1();

        bool show_reserved;

        RegisterTab *gpr_tab;
        RegisterTab *cop0_tab;
        RegisterTab *cop1_tab;
        wxNotebook *notebook;
};

#endif // REGISTERPANEL_H

