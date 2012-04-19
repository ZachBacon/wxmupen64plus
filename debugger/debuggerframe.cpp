#include "debuggerframe.h"

#include <wx/aui/framemanager.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dir.h>
#include <stdexcept>

#include "../mupen64plusplus/MupenAPI.h"
#include "../mupen64plusplus/osal_preproc.h"

#include "breakpointpanel.h"
#include "disasmpanel.h"
#include "memorypanel.h"
#include "debugconsole.h"
#include "registerpanel.h"
#include "memorysearch.h"

#include "breakpoint.h"
#include "colors.h"
#include "debugconfig.h"

DebuggerFrame *DebuggerFrame::g_debugger = 0;
wxPoint DebuggerFrame::g_aui_pos = wxDefaultPosition;

enum
{
    DEBUG_INIT = 1,
    DEBUG_UPDATE,
    DEBUG_VI
};

DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_DEBUG_EVENT);

enum
{
    break_panel_id = 5,
    disasm_panel_id,
    memory_panel_id,
    console_panel_id,
    register_panel_id,
    memsearch_panel_id,
    last_panel_id,
    state_run_id,
    state_pause_id,
    state_step_id,
    state_vibreak_id,
    last_state_id,
    run_on_boot_opt_id,
    runtime_update_opt_id,
    last_opt_id,
    pane_rename_id,
};

class SimpleTextEntryDialog : public wxDialog
{
    public:
        SimpleTextEntryDialog(wxWindow *parent, int id, const wxString &title, const wxString &defaultvalue) : wxDialog(parent, id, title)
        {
            wxPanel *panel = new wxPanel(this, -1), *buttonpanel = new wxPanel(panel, -1);
            wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL), *btnsizer = new wxBoxSizer(wxHORIZONTAL);
            value = new wxTextCtrl(panel, -1, defaultvalue, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            wxButton *ok = new wxButton(buttonpanel, wxID_OK, _("Ok"));
            wxButton *cancel = new wxButton(buttonpanel, wxID_CANCEL, _("Cancel"));

            sizer->Add(value, 0, wxEXPAND | wxALL, 3);
            sizer->Add(buttonpanel);
            btnsizer->Add(ok, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 3);
            btnsizer->Add(cancel, 0, wxEXPAND | wxBOTTOM | wxRIGHT, 3);

            panel->SetSizer(sizer);
            buttonpanel->SetSizer(btnsizer);
            wxSize fittingsize = sizer->ComputeFittingWindowSize(this);
            value->SetSize(fittingsize.x - 6, -1); // This prevents bug with SelectAll scrolling unnecessarily
            value->SelectAll();
            value->SetFocus();
            SetSize(fittingsize);

            value->Bind(wxEVT_CHAR, &SimpleTextEntryDialog::Ok, this);
        }
        void Ok(wxKeyEvent &evt)
        {
            int key = evt.GetKeyCode();
            if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER)
                EndModal(wxID_OK);
            else
                evt.Skip();
        }
        wxString GetValue()
        {
            return value->GetValue();
        }
        ~SimpleTextEntryDialog()
        {
        }
    private:
        wxTextCtrl *value;
};

DebuggerFrame::DebuggerFrame(wxWindow *parentwnd, int id) : wxFrame(parentwnd, id, _("Debugger"))
{
    if (g_debugger) // eek
        throw std::runtime_error("Attemped to create a second DebuggerFrame");

    output = 0;
    next_id = time(0);
    inited = false;
    vi_break = false;
    vi_count = 0;
    pc = 0;
    breakpoints = new BreakpointInterface;

    SetDebuggingCallbacks(&DebuggerFrame::DebuggerInit, &DebuggerFrame::DebuggerUpdate, &DebuggerFrame::DebuggerVi);
    InitDrawingValues();
    g_debugger = this;

    CreateMenubar();

    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuClose, this, wxID_CLOSE);
    Bind(wxEVT_CLOSE_WINDOW, &DebuggerFrame::Close, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuAddPanel, this, break_panel_id, last_panel_id - 1);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuState, this, state_run_id, last_state_id - 1);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuOption, this, run_on_boot_opt_id, last_opt_id - 1);
    Bind(wxEVT_AUI_PANE_CLOSE, &DebuggerFrame::PaneClosed, this);
    Bind(wxMUPEN_DEBUG_EVENT, wxCommandEventHandler(DebuggerFrame::ProcessCallback), this, 1, 3);
    Bind(wxEVT_RIGHT_UP, &DebuggerFrame::PaneTitleRClick, this);
}

void DebuggerFrame::GameClosed()
{
    if (!g_debugger)
        return;
    g_debugger->SaveGameValues();
}

void DebuggerFrame::Delete()
{
    if (!g_debugger)
        return;
    g_debugger->SaveConfig();
    g_debugger->SaveGameValues();
    g_debugger->Destroy();
}

void DebuggerFrame::Close(wxCloseEvent &evt)
{
    Delete();
}

DebuggerFrame::~DebuggerFrame()
{
    aui->UnInit();
    delete aui;
    delete breakpoints;
    g_debugger = 0;
}

DebugPanel *DebuggerFrame::AddPanel(int type, const wxString &name, DebugConfigSection &config)
{
    wxAuiPaneInfo info;
    info.Top();
    DebugPanel *panel;

    info.Name(name);
    switch (type)
    {
        case break_panel_id:
        {
            panel = new BreakpointPanel(this, -1, type, config);
            info.Caption(_("Breakpoints"));
            info.BestSize(wxSize(150, 400));
        }
        break;
        case disasm_panel_id:
        {
            panel = new DisasmPanel(this, -1, type, config);
            info.Caption(_("Disassembly"));
            info.BestSize(wxSize(350, 400));
            disasmpanels.push_back((DisasmPanel *)panel);
        }
        break;
        case memory_panel_id:
        {
            panel = new MemoryPanel(this, -1, type, config);
            info.Caption(_("Memory"));
            info.BestSize(wxSize(350, 400));
            mempanels.push_back((MemoryPanel *)panel);
        }
        break;
        case console_panel_id:
        {
            panel = new DebugConsole(this, -1, type);
            info.Caption(_("Console"));
            info.BestSize(wxSize(300, 500));
        }
        break;
        case register_panel_id:
        {
            panel = new RegisterPanel(this, -1, type, config);
            info.Caption(_("Registers"));
            info.BestSize(wxSize(300, 500));
        }
        break;
        case memsearch_panel_id:
        {
            panel = new MemSearchPanel(this, -1, type, config);
            info.Caption(_("Memory search"));
            info.BestSize(wxSize(150, 400));
        }
        break;
        default:
            return 0;
    }
    aui->AddPane(panel, info);
    return panel;
}

void DebuggerFrame::PaneClosed(wxAuiManagerEvent &evt)
{
    DebugPanel *pane = (DebugPanel *)evt.GetPane()->window;
    switch (pane->GetType())
    {
        case disasm_panel_id:
            for (auto it = disasmpanels.begin(); it != disasmpanels.end(); ++it)
            {
                if (*it == pane)
                {
                    disasmpanels.erase(it);
                    return;
                }
            }
            Print("Error: Did not have closed pane in disasmpanels");
        break;
        case memory_panel_id:
            for (auto it = mempanels.begin(); it != mempanels.end(); ++it)
            {
                if (*it == pane)
                {
                    mempanels.erase(it);
                    return;
                }
            }
            Print("Error: Did not have closed pane in mempanels");
        break;
        case console_panel_id:
        if (pane == output)
            output = 0;
        break;
    }
}

bool DebuggerFrame::LoadAui(const wxString &perspective_, DebugConfigIn *config)
{
    wxString perspective = perspective_;
    aui = new wxAuiManager(this);

    // I couldn't figure out a better way to do this
    // Maybe this should've just been hardcoded
    if (!g_aui_pos.IsFullySpecified())
    {
        wxPanel testpanel(this, -1);
        aui->AddPane(&testpanel, wxTOP);
        aui->Update();
        wxAuiPaneInfo &info = aui->GetPane(&testpanel);
        g_aui_pos.x = info.rect.x;
        g_aui_pos.y = info.rect.y;
        aui->DetachPane(&testpanel);
    }

    DebugConfigSection config_section;

    while (config->GetNextSection(&config_section))
    {
        char name[32];
        strncpy(name, config_section.name, 32);
        char *type_end = strchr(name, '_');
        if (!type_end)
            continue;
        type_end[0] = 0;

        if (osal_insensitive_strcmp(name, "Memory") == 0)
            AddPanel(memory_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "Disasm") == 0)
            AddPanel(disasm_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "Breaks") == 0)
            AddPanel(break_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "Console") == 0)
            AddPanel(console_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "MainConsole") == 0)
            output = (DebugConsole *)AddPanel(console_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "Registers") == 0)
            AddPanel(register_panel_id, config_section.name, config_section);
        else if (osal_insensitive_strcmp(name, "MemSearch") == 0)
            AddPanel(memsearch_panel_id, config_section.name, config_section);
    }
    aui->LoadPerspective(perspective);
    return true;
}

void DebuggerFrame::MenuAddPanel(wxCommandEvent &evt)
{
    int type = evt.GetId();
    wxString name;
    switch (type)
    {
        case break_panel_id:
            name = "Breaks";
        break;
        case disasm_panel_id:
            name = "Disasm";
        break;
        case memory_panel_id:
            name = "Memory";
        break;
        case console_panel_id:
            if (!output)
                name = "MainConsole";
            else
                name = "Console";
        break;
        case register_panel_id:
            name = "Registers";
        break;
        case memsearch_panel_id:
            name = "MemSearch";
        break;
        default:
            return;
    }
    name << "_" << ++next_id;

    DebugConfigSection empty_sect;
    empty_sect.num_values = 0;
    wxPanel *panel = AddPanel(type, name, empty_sect);
    if (!output && type == console_panel_id)
        output = (DebugConsole *)panel;
    aui->Update();
}

DebugPanel *DebuggerFrame::PaneTitleHitTest(const wxPoint &pos)
{
    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    uint32_t count = panes.GetCount();
    for (uint32_t i = 0; i < count; i++)
    {
        wxAuiPaneInfo &info = panes.Item(i);
        if (info.IsFloating())
            continue;
        wxRect panetitle_rect, pane_rect = info.rect;
        panetitle_rect = pane_rect;
        panetitle_rect.x = pane_rect.x - g_aui_pos.x;
        panetitle_rect.y = pane_rect.y - g_aui_pos.y;
        panetitle_rect.width = pane_rect.width + g_aui_pos.x * 2;
        panetitle_rect.height = g_aui_pos.y;
        if (panetitle_rect.Contains(pos))
            return (DebugPanel *)info.window;
    }
    return 0;
}

void DebuggerFrame::PaneTitleRClick(wxMouseEvent &evt)
{
    selectedpane = PaneTitleHitTest(evt.GetPosition());
    if (!selectedpane)
        return;

    wxMenu menu;
    wxMenuItem *rename;
    rename = new wxMenuItem(&menu, pane_rename_id, _("Rename.."));
    menu.Append(rename);
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::PaneTitleEvent, this);
    PopupMenu(&menu);
}


void DebuggerFrame::PaneTitleEvent(wxCommandEvent &evt)
{
    if (evt.GetId() == pane_rename_id)
    {
        wxAuiPaneInfo &info = aui->GetPane(selectedpane);
        SimpleTextEntryDialog dlg(this, -1, _("Rename"), info.caption);
        if (dlg.ShowModal() == wxID_OK)
        {
            info.Caption(dlg.GetValue());
            aui->Update();
        }
    }
}

bool DebuggerFrame::AddBreakpoint(Breakpoint *bpt)
{
    if (!breakpoints->Add(bpt))
        return false;

    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->BreakpointUpdate(bpt, BREAK_ADDED, true);
    }
    return true;
}

void DebuggerFrame::DeleteBreakpoint(Breakpoint *bpt, bool last_update)
{
    breakpoints->Delete(bpt);

    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->BreakpointUpdate(bpt, BREAK_REMOVED, last_update);
    }

}

void DebuggerFrame::EnableBreakpoint(Breakpoint *bpt, bool enable, bool last_update)
{
    if (enable == bpt->IsEnabled())
        return;
    if (enable)
        breakpoints->Enable(bpt);
    else
        breakpoints->Disable(bpt);

    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->BreakpointUpdate(bpt, BREAK_CHANGED, last_update);
    }
}

bool DebuggerFrame::EditBreakpoint(Breakpoint *bpt, const wxString &name, uint32_t address, int length, char type)
{
    if (!breakpoints->Update(bpt, name, address, length, type))
        return false;
    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->BreakpointUpdate(bpt, BREAK_CHANGED, true);
    }
    return true;
}

Breakpoint *DebuggerFrame::FindBreakpoint(uint32_t address, uint32_t length)
{
    return breakpoints->Find(address, length);
}

std::unique_ptr<Breakpoint *[]> DebuggerFrame::FindBreakpointsByName(const wxString &name, int *amt)
{
    return breakpoints->FindByName(name, amt);
}

const BreakContainer *DebuggerFrame::GetBreakpoints()
{
     return breakpoints->GetAllBreakpoints();
}

void DebuggerFrame::SaveConfig()
{
    DebugConfigOut config(wxString::Format("%swxmupen-debugger.cfg", GetUserConfigPath()));

    config.WriteComment("This file does not need to be modified by hand, everything should be editable through GUI.");
    config.WriteComment("If you wish to do it though, be careful, as a single syntax error will cause rest of the file to be ignored.");
    config.WriteComment("Also, [Main] has to be the first section or else nothing will be loaded.\n");

    DebugConfigSection section;
    section.name = "Main";
    section.num_values = 5;

    wxString perspective_string = aui->SavePerspective();
    section.keys[0] = "Layout";
    section.values[0] = perspective_string.c_str();

    wxRect rect = GetRect();
    section.keys[1] = "Window";
    char buf[32];
    sprintf(buf, "%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);
    section.values[1] = buf;

    section.keys[2] = "Maximized";
    (IsMaximized()) ? (section.values[2] = "True") : (section.values[2] = "False");

    section.keys[3] = "RunOnBoot";
    (run_on_boot != 0) ? (section.values[3] = "True") : (section.values[3] = "False");

    section.keys[4] = "RuntimeUpdate";
    (runtime_update) ? (section.values[4] = "True") : (section.values[4] = "False");

    config.WriteSection(section);

    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        DebugPanel *panel = (DebugPanel *)(panes.Item(i).window);
        section.name = panes.Item(i).name;
        section.num_values = 0;
        panel->SaveConfig(section);
        config.WriteSection(section);
    }
}

void DebuggerFrame::LoadConfig()
{
    DebugConfigIn config(wxString::Format("%swxmupen-debugger.cfg", GetUserConfigPath()));
    DebugConfigSection sect;
    if(!config.GetNextSection(&sect))
    {
        Print("Config error");
        run_on_boot = false;
        runtime_update = true;
        LoadAui("", 0);
    }
    else if (osal_insensitive_strcmp(sect.name, "Main") == 0)
    {
        if (osal_insensitive_strcmp(sect.GetValue("Maximized", "False"), "True") == 0)
            Maximize();
        else
        {
            wxRect rect;
            const char *value = sect.GetValue("Window", "100,100,500,500");
            sscanf(value, "%d,%d,%d,%d", &rect.x, &rect.y, &rect.width, &rect.height);
            SetSize(rect);
        }

        if (osal_insensitive_strcmp(sect.GetValue("RunOnBoot", "False"), "True") == 0)
            run_on_boot = true;
        else
            run_on_boot = false;

        if (osal_insensitive_strcmp(sect.GetValue("RuntimeUpdate", "True"), "True") == 0)
            runtime_update = true;
        else
            runtime_update = false;

        LoadAui(sect.GetValue("Layout", ""), &config);
    }
    else
    {
        run_on_boot = false;
        runtime_update = true;
        LoadAui("", 0);
    }

    run_on_boot_menu->Check(run_on_boot == 1);
    runtime_update_menu->Check(runtime_update);

    if (GetDebugState(M64P_DBG_CPU_DYNACORE) == 2)
        Print("WARNING: Dynamic recompiler limits debugging features, please use interpreter");
}

void DebuggerFrame::LoadGameValues()
{
    m64p_rom_settings rom_settings;
    getRomSettings(&rom_settings);

    wxString game_values_name, game_values_path = GetUserDataPath();
    game_values_path.Printf("%sdebugger", GetUserDataPath());
    game_values_name.Printf("%s/%s.ids", game_values_path, rom_settings.goodname);
    if (!wxDir::Exists(game_values_path))
        wxDir::Make(game_values_path);

    DebugConfigIn vals(game_values_name);
    if (vals.IsOk())
    {
        DebugConfigSection section;
        while(vals.GetNextSection(&section) == true)
        {
            if (osal_insensitive_strcmp(section.name, "breakpoint") == 0 && section.num_values >= 4)
            {
                uint32_t address, length, type;
                wxString name;

                name = wxString::FromUTF8(section.GetValue("name", ""));
                address = strtoul(section.GetValue("address", "0"), 0, 16);
                length = strtoul(section.GetValue("length", "1"), 0, 16);
                const char *type_str = section.GetValue("type", "x");
                while (type_str[0] != 0)
                {
                    if (type_str[0] == 'x')
                        type |= BREAK_TYPE_EXECUTE;
                    else if (type_str[0] == 'r')
                        type |= BREAK_TYPE_READ;
                    else if (type_str[0] == 'w')
                        type |= BREAK_TYPE_WRITE;
                    type_str++;
                }

                Breakpoint *bpt = new Breakpoint(name, address, length, type);
                AddBreakpoint(bpt);
                if (!osal_insensitive_strcmp(section.GetValue("enabled", "true"), "true") == 0)
                    EnableBreakpoint(bpt, false);
            }
        }
    }
}

void DebuggerFrame::SaveGameValues()
{
    m64p_rom_settings rom_settings;
    getRomSettings(&rom_settings);
    wxString game_values_name;
    game_values_name.Printf("%sdebugger/%s.ids", GetUserDataPath(), rom_settings.goodname);

    DebugConfigOut vals(game_values_name);
    DebugConfigSection section;
    section.name = "Breakpoint";
    section.keys[0] = "Name";
    section.keys[1] = "Address";
    section.keys[2] = "Length";
    section.keys[3] = "Type";
    section.keys[4] = "Enabled";
    section.num_values = 5;
    auto breaks = breakpoints->GetAllBreakpoints();
    for (auto it = breaks->begin(); it != breaks->end(); ++it)
    {
        char addr[16], len[16], type_str[4];
        int type_count = 0, type;
        Breakpoint *bpt = it->second;
        sprintf(addr, "%X", bpt->GetAddress());
        sprintf(len, "%X", bpt->GetLength());

        type = bpt->GetType();
        if (type & BREAK_TYPE_EXECUTE)
        {
            type_str[type_count] = 'x';
            type_count++;
        }
        if (type & BREAK_TYPE_READ)
        {
            type_str[type_count] = 'r';
            type_count++;
        }
        if (type & BREAK_TYPE_WRITE)
        {
            type_str[type_count] = 'w';
            type_count++;
        }
        type_str[type_count] = 0;

        section.values[0] = bpt->GetName().ToUTF8();
        section.values[1] = addr;
        section.values[2] = len;
        section.values[3] = type_str;
        if (bpt->IsEnabled())
            section.values[4] = "True";
        else
            section.values[4] = "False";

        vals.WriteSection(section);
    }
}

void DebuggerFrame::MenuAppendMemoryFollow(wxMenu *menu)
{
    int size = mempanels.size();
    if (!size)
        return;
    if (size == 1)
    {
        wxMenuItem *jmp = new wxMenuItem(menu, MEMJMP_ID_START, _("Memory follow"));
        menu->Append(jmp);
    }
    else
    {
        wxMenu *submenu = new wxMenu;
        for (int i = 0; i < size && i < PANELJMP_MAX_ID_PER_TYPE; i++)
        {
            wxMenuItem *jmp = new wxMenuItem(menu, MEMJMP_ID_START + i, aui->GetPane(mempanels[i]).caption);
            submenu->Append(jmp);
        }
        menu->AppendSubMenu(submenu, _("Memory follow"));
    }
}

void DebuggerFrame::MenuAppendDisasmFollow(wxMenu *menu)
{
    int size = disasmpanels.size();
    if (!size)
        return;
    if (size == 1)
    {
        wxMenuItem *jmp = new wxMenuItem(menu, ASMJMP_ID_START, _("Disassembly follow"));
        menu->Append(jmp);
    }
    else
    {
        wxMenu *submenu = new wxMenu;
        for (int i = 0; i < size && i < PANELJMP_MAX_ID_PER_TYPE; i++)
        {
            wxMenuItem *jmp = new wxMenuItem(menu, ASMJMP_ID_START + i, aui->GetPane(disasmpanels[i]).caption);
            submenu->Append(jmp);
        }
        menu->AppendSubMenu(submenu, _("Disassembly follow"));
    }
}

bool DebuggerFrame::DoFollow(int id, uint32_t address)
{
    if (id < ASMJMP_ID_START || id >PANELJMP_ID_LAST)
        return false;

    if (id > MEMJMP_ID_START)
        mempanels[id % PANELJMP_MAX_ID_PER_TYPE]->Goto(address);
    else // asm
        disasmpanels[id]->Goto(address, 0);

    return true;
}

void DebuggerFrame::CreateMenubar()
{
    wxMenu *filemenu = new wxMenu, *viewmenu = new wxMenu, *statemenu = new wxMenu, *optmenu = new wxMenu;

    filemenu->Append(wxID_CLOSE, _("&Close\tCtrl-Q"));

    run = new wxMenuItem(statemenu, state_run_id, _("Run\tF9"), wxEmptyString, wxITEM_CHECK);
    pause = new wxMenuItem(statemenu, state_pause_id, _("Pause\tF12"), wxEmptyString, wxITEM_CHECK);
    wxMenuItem *separator1 = new wxMenuItem(statemenu);
    wxMenuItem *step = new wxMenuItem(statemenu, state_step_id, _("Step\tF7"));

    run_on_boot_menu = new wxMenuItem(optmenu, run_on_boot_opt_id, _("Run on boot"), wxEmptyString, wxITEM_CHECK);
    runtime_update_menu = new wxMenuItem(optmenu, runtime_update_opt_id, _("Update values while running"), wxEmptyString, wxITEM_CHECK);

    statemenu->Append(run);
    statemenu->Append(pause);
    statemenu->Append(separator1);
    statemenu->Append(step);

    viewmenu->Append(console_panel_id, _("Console"));
    viewmenu->Append(break_panel_id, _("Breakpoints\tCtrl-B"));
    viewmenu->Append(register_panel_id, _("Registers\tCtrl-R"));
    viewmenu->Append(disasm_panel_id, _("Disassembly\tCtrl-D"));
    viewmenu->Append(memory_panel_id, _("Memory\tCtrl-M"));
    viewmenu->Append(memsearch_panel_id, _("Memory search\tCtrl-S")); // Todo: maybe these hotkeys aren't the best possibilities?

    optmenu->Append(run_on_boot_menu);
    optmenu->Append(runtime_update_menu);

    wxMenuBar *menubar = new wxMenuBar;
    menubar->Append(filemenu, _("File"));
    menubar->Append(statemenu, _("State"));
    menubar->Append(viewmenu, _("View"));
    menubar->Append(optmenu, _("Options"));

    // Game starts as paused by default, this might be overridden by run_on_boot though
    pause->Check(true);
    run->Check(false);

    SetMenuBar(menubar);
}

void DebuggerFrame::MenuState(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case state_run_id:
            Run();
        break;
        case state_pause_id:
            Pause();
        break;
        case state_step_id:
            Step();
        break;
        case state_vibreak_id:
            ViBreak();
        break;
        default:
        return;
    }
}

void DebuggerFrame::MenuOption(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case run_on_boot_opt_id:
            if (run_on_boot != 0)
                run_on_boot = 0;
            else
                run_on_boot = 2;
        break;
        case runtime_update_opt_id:
            if (runtime_update)
                runtime_update = false;
            else
                runtime_update = true;

        break;
        default:
        break;
    }
}

void DebuggerFrame::Run()
{
    run->Check(true);
    pause->Check(false);
    SetRunState(2);
    running = true;
    DebuggerStep();
}

void DebuggerFrame::Step()
{
    DebuggerStep();
}

void DebuggerFrame::Pause()
{
    SetRunState(0);
}

void DebuggerFrame::ViBreak()
{
    vi_break = true;
    Run();
}

void DebuggerFrame::Print(const wxString &msg)
{
    if (output)
        output->Print(msg);
    else
        printf("Debugger: %s\n", (const char *)msg);
}

void DebuggerFrame::MenuClose(wxCommandEvent &evt)
{
    Delete();
}

void DebuggerFrame::UpdatePanels(bool vi)
{
    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->Update(vi);
    }
}

void DebuggerFrame::RefreshPanels()
{
    UpdatePanels(running == true);
}

wxString DebuggerFrame::GetBreakReason(uint32_t pc)
{
    Breakpoint *bpt = FindBreakpoint(pc);
    if (bpt && (bpt->GetType() & BREAK_TYPE_EXECUTE))
        return wxString::Format("Hit breakpoint \"%s\"", bpt->GetName());
    else
        return wxString::Format("Paused at %08X", pc);
}

void DebuggerFrame::ProcessCallback(wxCommandEvent &evt)
{
    switch (evt.GetId())
    {
        case DEBUG_INIT:
        {
            if (!inited)
            {
                LoadConfig();
                Show();
                inited = true;
            }
            Reset();
        }
        break;
        case DEBUG_UPDATE:
        {
            vi_break = false;
            if (run_on_boot == 1)
            {
                run_on_boot = 2;
                Run();
            }
            else
            {
                pc = evt.GetInt();
                if (running)
                    Print(GetBreakReason(pc));
                pause->Check(true);
                run->Check(false);
                running = false;
                UpdatePanels();
                Raise();
            }
        }
        break;
        case DEBUG_VI:
        {
            vi_count++;
            if (vi_break)
                Pause();
            // Update only at every 10th vi, decreases the unnecessary flickering with default wx controls
            else if (runtime_update && (vi_count % 10) == 0)
                UpdatePanels(true);
        }
        break;
        default:
            return;
    }
}

void DebuggerFrame::DebuggerInit()
{
    wxCommandEvent evt(wxMUPEN_DEBUG_EVENT, DEBUG_INIT);
    g_debugger->AddPendingEvent(evt);
}

void DebuggerFrame::DebuggerUpdate(unsigned int pc)
{
    wxCommandEvent evt(wxMUPEN_DEBUG_EVENT, DEBUG_UPDATE);
    evt.SetInt(pc);
    g_debugger->AddPendingEvent(evt);
}

void DebuggerFrame::DebuggerVi()
{
    if (!g_debugger)
        return;
    wxCommandEvent evt(wxMUPEN_DEBUG_EVENT, DEBUG_VI);
    g_debugger->AddPendingEvent(evt);
}

void DebuggerFrame::Reset()
{
    if (run_on_boot == 2)
        run_on_boot = 1;

    breakpoints->Clear();

    wxAuiPaneInfoArray panes = aui->GetAllPanes();
    for (uint32_t i = 0; i < panes.GetCount(); i++)
    {
        ((DebugPanel *)(panes.Item(i).window))->Reset();
    }
    LoadGameValues();
    // TODO--
}

