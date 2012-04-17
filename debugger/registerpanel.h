#ifndef REGISTERPANEL_H
#define REGISTERPANEL_H

#include "debugpanel.h"
#include <vector>
#include <wx/scrolwin.h>

class wxAuiNotebook;
class wxAuiNotebookEvent;
class wxTextCtrl;
class RegisterTab;
class wxStaticText;

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

class SingleRegister : public wxPanel
{
    public:
        SingleRegister(RegisterTab *parent, int id, const char *name, RegisterType type, const wxPoint &pos, int reg_name_len);
        ~SingleRegister();
        uint64_t GetInt();
        double GetFloat();

        void SetInt(uint64_t value);
        void SetFloat(float value);
        void SetDouble(double value);
        void ResetValue();
        void ValueChanged(wxCommandEvent &evt);
        void ValueChanged();

        void RClickMenu(wxMouseEvent &evt);
        void RClickEvent(wxCommandEvent &evt);

        void SetType(RegisterType new_type);

    private:
        RegisterTab *parent;
        bool setting_value;
        wxAny original_value;
        wxTextCtrl *value;
        wxTextCtrl *value2;
        RegisterType type;
};

class RegisterTab : public wxScrolledWindow
{
    public:
        RegisterTab(wxWindow *parent, int id);
        virtual ~RegisterTab();

        void Size(wxSizeEvent &evt);
        void Resize(int new_cols);

        void Append(const char *name, RegisterType type, int id = -1);
        int GetCols() { return cols; }
        int GetRows() { return rows; }

        virtual void ValueChanged(int id, const wxAny &value) = 0;
        virtual void Update() = 0;

        virtual void RClickMenu() {}
        virtual void MenuClick(wxCommandEvent &evt) {}

    protected:
        int reg_name_len;
        int reg_basewidth;
        virtual void Reorder() = 0;
        virtual int GetAmount() = 0;
        wxPoint CalcItemPos(int index);

    private:
        char cols;
        char rows;
};

class GprTab : public RegisterTab
{
    public:
        GprTab(wxWindow *parent, int id);
        ~GprTab();

        void Update();
        void ValueChanged(int id, const wxAny &value);

    protected:
        void Reorder();
        int GetAmount() { return 32; }

    private:
        SingleRegister *registers[32];
        uint64_t *raw_registers;
};

class Cop0Tab : public RegisterTab
{
    public:
        Cop0Tab(wxWindow *parent, int id);
        ~Cop0Tab();

        void Update();
        void ValueChanged(int id, const wxAny &value);
        void RClickMenu();
        void RClickEvent(wxCommandEvent &evt);

    protected:
        void Reorder();
        int GetAmount();

    private:
        bool show_reserved;
        SingleRegister *registers[32];
        uint32_t *raw_registers;
};

class Cop1Tab : public RegisterTab
{
    public:
        Cop1Tab(wxWindow *parent, int id);
        ~Cop1Tab();

        void Update();
        void UpdateRegs();
        void ValueChanged(int id, const wxAny &value);
        void RClickMenu();
        void RClickEvent(wxCommandEvent &evt);

    protected:
        void Reorder();
        int GetAmount();

    private:
        SingleRegister *registers[32];
        int mode;
        bool additional_regs;
        float **raw_registers_simple;
        double **raw_registers_double;
        uint64_t *raw_registers_raw;
};

class ViTab : public RegisterTab
{
    public:
        ViTab(wxWindow *parent, int id);
        ~ViTab();

        void Update();
        void ValueChanged(int id, const wxAny &value);

    protected:
        void Reorder();
        int GetAmount() { return 15; }

    private:
        SingleRegister *registers[15];
        uint32_t *raw_registers;
};

class AiTab : public RegisterTab
{
    public:
        AiTab(wxWindow *parent, int id);
        ~AiTab();

        void Update();
        void ValueChanged(int id, const wxAny &value);

    protected:
        void Reorder();
        int GetAmount() { return 10; }

    private:
        SingleRegister *registers[10];
        uint32_t *raw_registers;
};

class PiTab : public RegisterTab
{
    public:
        PiTab(wxWindow *parent, int id);
        ~PiTab();

        void Update();
        void ValueChanged(int id, const wxAny &value);

    protected:
        void Reorder();
        int GetAmount() { return 13; }

    private:
        SingleRegister *registers[13];
        uint32_t *raw_registers;
};

class RiSiTab : public RegisterTab
{
    public:
        RiSiTab(wxWindow *parent, int id);
        ~RiSiTab();

        void Update();
        void ValueChanged(int id, const wxAny &value);

    protected:
        void Reorder();
        int GetAmount() { return 10; }

        wxPoint CalcItemPos(int index);
        wxPoint CalcTextPos(int index);

    private:
        SingleRegister *registers[8];
        uint32_t *raw_ri_registers;
        uint32_t *raw_si_registers;
        wxStaticText *ri_text;
        wxStaticText *si_text;
};

class RegisterPanel : public DebugPanel
{
    public:
        RegisterPanel(DebuggerFrame *parent, int id, int type);
        virtual ~RegisterPanel();

        void Update(bool vi);
        void TabRClick(wxAuiNotebookEvent &evt);

    private:
        uint32_t updated_panels;
        wxAuiNotebook *notebook;
};

#endif // REGISTERPANEL_H

