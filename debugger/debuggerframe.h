#ifndef DEBUGGERFRAME_H
#define DEBUGGERFRAME_H

#include <wx/frame.h>
#include <wx/aui/framemanager.h>
#include <set>

#include "main.h"

DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_DEBUG_EVENT, -1);

class DebugConsole;
class DebugPanel;

enum BreakpointStatus
{
    BREAK_STATUS_UNKNOWN = 0,
    BREAK_STATUS_ACTIVE,
    BREAK_STATUS_DISABLED,
    BREAK_STATUS_WATCH
};


#define BREAK_TYPE_EXECUTE 0x1
#define BREAK_TYPE_READ 0x2
#define BREAK_TYPE_WRITE 0x4
#define BREAK_TYPE_ALL BREAK_TYPE_EXECUTE | BREAK_TYPE_READ | BREAK_TYPE_WRITE


struct Breakpoint
{
    wxString name;
    int id;
    uint32_t offset;
    char status;
    char type;
};

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
        void AddPanel(wxCommandEvent &evt);
        void RemovePanel(DebugPanel *panel);
        void ConsoleClosed(DebugConsole *console);

        void State(wxCommandEvent &evt);
        void Run();
        void Step();
        void Pause();

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
        std::set<DebugPanel *> panels;
        bool runtime_update;
        void UpdatePanels(bool vi = false);

        void CreateMenubar();
        wxMenuItem *run;
        wxMenuItem *pause;

        bool inited;
        void LoadConfig();
        bool LoadAui(const wxString &perspective);
        bool run_on_boot;

        wxAuiManager *aui;
        ConfigSection *config;
};


#endif // DEBUGGERFRAME_H
