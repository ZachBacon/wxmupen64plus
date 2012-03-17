#ifndef DEBUGGERFRAME_H
#define DEBUGGERFRAME_H

#include <wx/frame.h>
#include <wx/aui/framemanager.h>
#include <set>

#include "main.h"

DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_DEBUG_EVENT, -1);

class DebugConsole;
class DebugPanel;
class Breakpoint;

// This code assumes that there won't be multiple DebuggerFrames,
// as it makes implementing debug callbacks a lot easier..
// Why would there be anyways?

class DebuggerFrame : public wxFrame
{
    public:
        DebuggerFrame(wxWindow *parentwnd = 0, int id = -1);
        virtual ~DebuggerFrame();

        void Reset();

        void Close(wxCloseEvent &evt);
        void MenuClose(wxCommandEvent &evt);
        void MenuAddPanel(wxCommandEvent &evt);
        void MenuState(wxCommandEvent &evt);
        void MenuOption(wxCommandEvent &evt);

        void ConsoleClosed(DebugConsole *console);

        void Run();
        void Step();
        void Pause();
        void ViBreak();
        void RefreshPanels();

        uint32_t GetPc() { return pc; }

        DebugConsole *GetMainOutput() { return output; }
        void SetMainOutput(DebugConsole *output_) { output = output_; }

        void NewBreakpoint(Breakpoint *data);
        void DeleteBreakpoint(Breakpoint *data);
        Breakpoint *FindBreakpoint(int id);
        Breakpoint *FindBreakpoint(wxString name);

        void ProcessCallback(wxCommandEvent &evt);

        void Print(wxString msg);
        void SaveConfig();

        static bool Exists() { return g_debugger != 0; }
        static void Delete();

    private:
        static void DebuggerInit();
        static void DebuggerUpdate(unsigned int pc);
        static void DebuggerVi();
        static DebuggerFrame *g_debugger;
        static m64p_handle debugger_config;

        int next_id;

        DebugPanel *AddPanel(int type, wxString &name, int id = 0);
        DebugConsole *output;
        bool vi_break;
        char vi_count;
        bool inited;
        bool running;
        uint32_t pc;
        void UpdatePanels(bool vi = false);

        void CreateMenubar();
        wxMenuItem *run;
        wxMenuItem *pause;

        void LoadConfig();
        bool LoadAui(const wxString &perspective);
        bool runtime_update;
        char run_on_boot; // 0 = nope; 1 = yea; 2 = yea, done

        wxMenuItem *run_on_boot_menu;
        wxMenuItem *runtime_update_menu;

        wxAuiManager *aui;
        ConfigSection *config;
};


#endif // DEBUGGERFRAME_H
