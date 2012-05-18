/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus debugger                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Markus Heikkinen                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEBUGGERFRAME_H
#define DEBUGGERFRAME_H

#include <wx/frame.h>
#include <unordered_map>
#include <vector>

#include "main.h"
#include "breakpoint.h"

#define PANELJMP_MAX_ID_PER_TYPE 50
#define ASMJMP_ID_START 300
#define MEMJMP_ID_START (ASMJMP_ID_START + PANELJMP_MAX_ID_PER_TYPE)
#define PANELJMP_ID_LAST (MEMJMP_ID_START + PANELJMP_MAX_ID_PER_TYPE)

DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_DEBUG_EVENT, -1);

class wxAuiManager;
class wxAuiManagerEvent;
class MemoryPanel;
class DisasmPanel;
class DebugConsole;
class DebugPanel;
class Breakpoint;
class BreakpointInterface;
class wxMenu;
class DebugConfigIn;
class DebugConfigSection;

// This code assumes that there won't be multiple DebuggerFrames,
// as it makes implementing debug callbacks a lot easier..
// Why would there be anyways?

class DebuggerFrame : public wxFrame
{
    public:
        DebuggerFrame(wxWindow *parentwnd = 0, int id = -1);
        virtual ~DebuggerFrame();

        void Reset();
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
        void Print(const wxString &msg);

        void MenuAppendMemoryFollow(wxMenu *menu);
        void MenuAppendDisasmFollow(wxMenu *menu);
        bool DoFollow(int id, uint32_t address);

        static bool Exists() { return g_debugger != 0; }
        static void Delete();
        static void GameClosed();

        void ProcessCallback(wxCommandEvent &evt);
        void Close(wxEvent &evt);
        void MenuAddPanel(wxCommandEvent &evt);
        void MenuState(wxCommandEvent &evt);
        void MenuOption(wxCommandEvent &evt);
        void MenuAuiMode(wxCommandEvent &evt);
        void PaneTitleRClick(wxMouseEvent &evt);
        void PaneTitleEvent(wxCommandEvent &evt);
        void PaneCloseEvent(wxAuiManagerEvent &evt);

    private:
        static void DebuggerInit();
        static void DebuggerUpdate(unsigned int pc);
        static void DebuggerVi();

        static DebuggerFrame *g_debugger;
        static wxPoint g_aui_pos;

        DebugPanel *AddPanel(int type, const wxString &name, DebugConfigSection &config);

        int next_id;
        DebugConsole *output;
        std::vector<MemoryPanel *> mempanels;
        std::vector<DisasmPanel *> disasmpanels;

        void UpdatePanels(bool vi = false);
        wxString GetBreakReason(uint32_t pc);

        bool vi_break;
        char vi_count;
        bool inited;
        bool running;
        uint32_t pc;
        BreakpointInterface *breakpoints;

        DebugPanel *PaneTitleHitTest(const wxPoint &pos);
        DebugPanel *selectedpane;

        void CreateMenubar();
        wxMenuItem *run;
        wxMenuItem *pause;
        wxMenuItem *run_on_boot_menu;
        wxMenuItem *runtime_update_menu;
        wxMenuItem *floatpanes_menu;
        wxMenuItem *lockpanes_menu;

        void LoadConfig();
        void SaveConfig();
        bool LoadAui(const wxString &perspective, DebugConfigIn *config);
        void LoadGameValues();
        void SaveGameValues();

        void SafeAuiUpdate();
        void PaneClosed(DebugPanel *pane);

        wxAuiManager *aui;
        uint8_t aui_mode; // 1 and larger = float new panes, 2 and larger = lock main window
        bool runtime_update;
        char run_on_boot; // 0 = nope; 1 = yea; 2 = yea, done
};


#endif // DEBUGGERFRAME_H
