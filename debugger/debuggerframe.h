#ifndef DEBUGGERFRAME_H
#define DEBUGGERFRAME_H

#include <wx/frame.h>
#include <unordered_map>

#include "main.h"
#include "breakpoint.h"

DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_DEBUG_EVENT, -1);

class wxAuiManager;
class DebugConsole;
class DebugPanel;
class Breakpoint;
class BreakpointInterface;

// This code assumes that there won't be multiple DebuggerFrames,
// as it makes implementing debug callbacks a lot easier..
// Why would there be anyways?

class DebuggerFrame : public wxFrame
{
    public:
        DebuggerFrame(wxWindow *parentwnd = 0, int id = -1);
        virtual ~DebuggerFrame();

        void Reset();

        void ConsoleClosed(DebugConsole *console);

        void Run();
        void Step();
        void Pause();
        void ViBreak();

        // These are practically BreakpointInterface interface :p
        // (With addition of telling all child panels about the changes)
        bool AddBreakpoint(Breakpoint *bpt);
        void DeleteBreakpoint(Breakpoint *bpt, bool refresh = true);
        Breakpoint *FindBreakpoint(uint32_t address, uint32_t length = 1);
        std::unique_ptr<Breakpoint *[]> FindBreakpointsByName(const wxString &name, int *amt);
        bool EditBreakpoint(Breakpoint *bpt, const wxString &name, uint32_t address, int length, char type);
        void EnableBreakpoint(Breakpoint *bpt, bool enable = true, bool refresh = true);
        const BreakContainer *GetBreakpoints();

        void RefreshPanels();

        uint32_t GetPc() { return pc; }

        DebugConsole *GetMainOutput() { return output; }
        void SetMainOutput(DebugConsole *output_) { output = output_; }

        void ProcessCallback(wxCommandEvent &evt);

        void Print(const wxString &msg);

        static bool Exists() { return g_debugger != 0; }
        static void Delete();
        static void GameClosed();

        void Close(wxCloseEvent &evt);
        void MenuClose(wxCommandEvent &evt);
        void MenuAddPanel(wxCommandEvent &evt);
        void MenuState(wxCommandEvent &evt);
        void MenuOption(wxCommandEvent &evt);
        void PaneTitleRClick(wxMouseEvent &evt);
        void PaneTitleEvent(wxCommandEvent &evt);

    private:
        static void DebuggerInit();
        static void DebuggerUpdate(unsigned int pc);
        static void DebuggerVi();
        static DebuggerFrame *g_debugger;
        static m64p_handle debugger_config;

        wxString GetBreakReason(uint32_t pc);

        static wxPoint g_aui_pos;

        int next_id;

        DebugPanel *AddPanel(int type, wxString &name, int id = 0);
        DebugConsole *output;
        bool vi_break;
        char vi_count;
        bool inited;
        bool running;
        uint32_t pc;
        void UpdatePanels(bool vi = false);
        BreakpointInterface *breakpoints;

        DebugPanel *PaneTitleHitTest(const wxPoint &pos);
        DebugPanel *selectedpane;

        void CreateMenubar();
        wxMenuItem *run;
        wxMenuItem *pause;

        void LoadConfig();
        void SaveConfig();
        bool LoadAui(const wxString &perspective);
        void LoadGameValues();
        void SaveGameValues();
        bool runtime_update;
        char run_on_boot; // 0 = nope; 1 = yea; 2 = yea, done

        wxMenuItem *run_on_boot_menu;
        wxMenuItem *runtime_update_menu;

        wxAuiManager *aui;
        ConfigSection *config;
};


#endif // DEBUGGERFRAME_H
