#ifndef REGISTERPANEL_H
#define REGISTERPANEL_H

#include "debugpanel.h"
#include <vector>

class wxNotebook;
class wxTextCtrl;
class wxGridSizer;

enum RegisterType
{
    REGISTER_INT64 = 1,
};

class SingleRegister : public wxPanel
{
    public:
        SingleRegister(wxWindow *parent, int id, const char *name, RegisterType type);
        ~SingleRegister();
        uint64_t GetInt();
        double GetFloat();

        void SetInt(uint64_t value);
        void SetFloat(double value);

    private:
        wxTextCtrl *value;
        RegisterType type;
};

class RegisterTab : public wxPanel
{
    public:
        RegisterTab(wxWindow *parent, int id);
        ~RegisterTab();

        void Append(const char *name, RegisterType type);

    private:
        wxGridSizer *sizer;
        std::vector<SingleRegister *> registers;
};

class RegisterPanel : public DebugPanel
{
    public:
        RegisterPanel(DebuggerFrame *parent, int id = -1);
        virtual ~RegisterPanel();

        void Update();

    private:
        RegisterTab *gpr_tab;
        RegisterTab *cop0_tab;
        RegisterTab *cop1_tab;
        wxNotebook *notebook;
};

#endif // REGISTERPANEL_H

