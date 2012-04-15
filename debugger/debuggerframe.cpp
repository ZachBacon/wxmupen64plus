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

extern ptr_ConfigOpenSection PtrConfigOpenSection; // lazy

DebuggerFrame *DebuggerFrame::g_debugger = 0;
m64p_handle DebuggerFrame::debugger_config = 0;
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
    next_id = 0;
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

DebugPanel *DebuggerFrame::AddPanel(int type, wxString &name, int id)
{
    wxAuiPaneInfo info;
    info.Top();
    DebugPanel *panel;
    if (!id)
        id = wxNewId();
    name << '_' << id;
    info.Name(name);
    switch (type)
    {
        case break_panel_id:
        {
            panel = new BreakpointPanel(this, id);
            info.Caption(_("Breakpoints"));
            info.BestSize(wxSize(150, 400));
            info.MinSize(wxSize(100, 150));
        }
        break;
        case disasm_panel_id:
        {
            panel = new DisasmPanel(this, id);
            info.Caption(_("Disassembly"));
            info.BestSize(wxSize(350, 400));
            info.MinSize(wxSize(350, 150));
        }
        break;
        case memory_panel_id:
        {
            panel = new MemoryPanel(this, id);
            info.Caption(_("Memory"));
            info.BestSize(wxSize(350, 400));
            info.MinSize(wxSize(350, 150));
        }
        break;
        case console_panel_id:
        {
            panel = new DebugConsole(this, id);
            info.Caption(_("Console"));
            info.BestSize(wxSize(300, 500));
            info.MinSize(wxSize(150, 100));
        }
        break;
        case register_panel_id:
        {
            panel = new RegisterPanel(this, id);
            info.Caption(_("Registers"));
            info.BestSize(wxSize(300, 500));
            info.MinSize(wxSize(150, 100));
        }
        break;
        case memsearch_panel_id:
        {
            panel = new MemSearchPanel(this, id);
            info.Caption(_("Memory search"));
            info.BestSize(wxSize(150, 400));
            info.MinSize(wxSize(300, 150));
        }
        break;
        default:
            return 0;
    }
    aui->AddPane(panel, info);
    return panel;
}

bool DebuggerFrame::LoadAui(const wxString &perspective_)
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

    size_t pos = 0, name_end, id_end;
    int id;
    wxString name;
    while ((pos = perspective.find("name=", pos)) != perspective.npos)
    {
        pos += 5;
        size_t state_pos = perspective.find("state=", pos);
        if (state_pos != perspective.npos)
        {
            const char *cstring = perspective.c_str() + state_pos + 6;
            uint32_t state = atoi(cstring);
            if (state & wxAuiPaneInfo::wxAuiPaneState::optionHidden) // If the pane is hidden, don't load it
                continue; // I don't get how they become hidden in the first place <.<
        }
        name_end = perspective.find('_', pos);
        name = perspective.substr(pos, name_end - pos);
        id = wxNewId();

        if (name == "Memory")
            AddPanel(memory_panel_id, name, id);
        else if (name == "Disasm")
            AddPanel(disasm_panel_id, name, id);
        else if (name == "Breaks")
            AddPanel(break_panel_id, name, id);
        else if (name == "Console")
            AddPanel(console_panel_id, name, id);
        else if (name == "MainConsole")
            output = (DebugConsole *)AddPanel(console_panel_id, name, id);
        else if (name == "Registers")
            AddPanel(register_panel_id, name, id);
        else if (name == "MemSearch")
            AddPanel(memsearch_panel_id, name, id);


        id_end = perspective.find_first_of(";|", name_end);
        char buf[16];
        sprintf(buf, "%d", id);
        perspective.replace(name_end + 1, id_end - name_end - 1, buf);
    }
    aui->LoadPerspective(perspective);
    RefreshPanels();
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
    wxPanel *panel = AddPanel(type, name);
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
    ptr_vector<ConfigSection>& config = wxGetApp().getConfig();
    for (int n = 0; n < config.size(); n++)
    {
        if (config[n]->m_section_name == "UI-wx-debugger")
        {
            ConfigParam *perspective = config[n]->getParamWithName("Layout");
            wxString perspective_string = aui->SavePerspective();
            perspective->setStringValue((const char *)perspective_string.c_str());

            char buf[32];
            wxRect rect = GetRect();
            sprintf(buf, "%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);
            config[n]->getParamWithName("Window")->setStringValue(buf);

            config[n]->getParamWithName("RunOnBoot")->setBoolValue(run_on_boot != 0);
            config[n]->getParamWithName("RuntimeUpdate")->setBoolValue(runtime_update);
        }
    }
}

void DebuggerFrame::LoadConfig()
{
    config = 0;
    unsigned int warnings = 0;
    ConfigParam *param;

    ptr_vector<ConfigSection>& conf = wxGetApp().getConfig();
    for (int n = 0; n < conf.size(); n++)
    {
        if (conf[n]->m_section_name == "UI-wx-debugger")
        {
            config = conf[n];
            debugger_config = conf[n]->m_handle;
        }
        else if (conf[n]->m_section_name == "Core")
        {
            if (conf[n]->getParamWithName("R4300Emulator")->getIntValue() == 2)
                warnings |= 0x1;
        }
    }
    if (!config)
    {
        if ((*PtrConfigOpenSection)("UI-wx-debugger", &debugger_config) != M64ERR_SUCCESS)
            throw std::runtime_error("Could not create debugger section in the config file.");

        config = new ConfigSection("UI-wx-debugger", debugger_config);
        conf.push_back(config);
    }

    param = config->getParamWithName("Window");
    if (!param)
    {
        char buf[32];
        wxRect rect = GetRect();
        sprintf(buf, "%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);
        config->addNewParam("Window", "Window position and size (x,y,w,h)", "", M64TYPE_STRING);
    }
    else
    {
        wxRect rect;
        // Can't do just "value = param->getStringValue().c_str()", since the string would be destructed before sscanf
        std::string str = param->getStringValue();
        const char *value = str.c_str();
        sscanf(value, "%d,%d,%d,%d", &rect.x, &rect.y, &rect.width, &rect.height);
        SetSize(rect);
    }

    param = config->getParamWithName("RunOnBoot");
    if (!param)
    {
        run_on_boot = false;
        config->addNewParam("RunOnBoot", "Runs the game when it is started", false, M64TYPE_BOOL);
    }

    else // I just don't like casting bool to a number and vice versa
    {
        if(param->getBoolValue())
            run_on_boot = 1;
        else
            run_on_boot = 0;
    }
    run_on_boot_menu->Check(run_on_boot == 1);

    param = config->getParamWithName("RuntimeUpdate");
    if (!param)
    {
        runtime_update = true;
        config->addNewParam("RuntimeUpdate", "Updates values while running", true, M64TYPE_BOOL);
    }
    else
        runtime_update = param->getBoolValue();
    runtime_update_menu->Check(runtime_update);


    param = config->getParamWithName("Layout");
    if (!param)
    {
        config->addNewParam("Layout", "Layout of the debugger", "", M64TYPE_STRING);
        LoadAui("");
    }
    else
    {
        LoadAui(param->getStringValue());
    }
    if (warnings & 0x1)
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
                uint32_t address = 0, length = 0, type = 0;
                wxString name;
                bool enabled = true;
                for (int i = 0; i < section.num_values; i++)
                {
                    if (osal_insensitive_strcmp(section.keys[i], "name") == 0)
                        name = wxString::FromUTF8(section.values[i]);
                    else if (osal_insensitive_strcmp(section.keys[i], "address") == 0)
                        address = strtoul(section.values[i], 0, 16);
                    else if (osal_insensitive_strcmp(section.keys[i], "length") == 0)
                        length = strtoul(section.values[i], 0, 16);
                    else if (osal_insensitive_strcmp(section.keys[i], "type") == 0)
                    {
                        const char *type_str = section.values[i];
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
                    }
                    else if (osal_insensitive_strcmp(section.keys[i], "enabled") == 0)
                        enabled = osal_insensitive_strcmp(section.values[i], "true") == 0;
                }
                if (!name || !length || !type)
                    continue;
                Breakpoint *bpt = new Breakpoint(name, address, length, type);
                AddBreakpoint(bpt);
                if (!enabled)
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

        vals.WriteSection(&section);
    }
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

void DebuggerFrame::ConsoleClosed(DebugConsole *console)
{
    if (console == output)
    {
        output = 0;
    }
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

