#include "debuggerframe.h"

#include <wx/menu.h>
#include <wx/panel.h>
#include <stdexcept>

#include "breakpointpanel.h"
#include "disasmpanel.h"
#include "memorypanel.h"
#include "debugconsole.h"
#include "registerpanel.h"
#include "../mupen64plusplus/MupenAPI.h"

extern ptr_ConfigOpenSection PtrConfigOpenSection; // lazy

DebuggerFrame *DebuggerFrame::g_debugger = 0;
m64p_handle DebuggerFrame::debugger_config = 0;

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
    last_panel_id,
    state_run_id,
    state_pause_id,
    state_step_id,
    state_vibreak_id,
    last_state_id,
    run_on_boot_opt_id,
    runtime_update_opt_id,
    last_opt_id
};

DebuggerFrame::DebuggerFrame(wxWindow *parentwnd, int id) : wxFrame(parentwnd, id, _("Debugger"))
{
    if (g_debugger) // eek
        throw std::runtime_error("Attemped to create a second DebuggerFrame");

    output = 0;
    next_id = 0;
    inited = false;
    vi_break = false;

    SetDebuggingCallbacks(&DebuggerFrame::DebuggerInit, &DebuggerFrame::DebuggerUpdate, &DebuggerFrame::DebuggerVi);
    g_debugger = this;

    CreateMenubar();

    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuClose, this, wxID_CLOSE);
    Bind(wxEVT_CLOSE_WINDOW, &DebuggerFrame::Close, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuAddPanel, this, break_panel_id, last_panel_id - 1);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuState, this, state_run_id, last_state_id - 1);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &DebuggerFrame::MenuOption, this, run_on_boot_opt_id, last_opt_id - 1);
    Bind(wxMUPEN_DEBUG_EVENT, wxCommandEventHandler(DebuggerFrame::ProcessCallback), this, 1, 3);
}

void DebuggerFrame::Delete()
{
    if (!g_debugger)
        return;
    g_debugger->SaveConfig();
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
    size_t pos = perspective.find("name=", 0), name_end, id_end;
    int id;
    wxString name;
    while (pos != perspective.npos)
    {
        pos += 5;
        name_end = perspective.find('_', pos);
        name = perspective.substr(pos, name_end - pos);
        id = wxNewId();

        if(name == "Memory")
            AddPanel(memory_panel_id, name, id);
        else if(name == "Disasm")
            AddPanel(disasm_panel_id, name, id);
        else if(name == "Breaks")
            AddPanel(break_panel_id, name, id);
        else if(name == "Console")
            AddPanel(console_panel_id, name, id);
        else if(name == "MainConsole")
            output = (DebugConsole *)AddPanel(console_panel_id, name, id);
        else if(name == "Registers")
            AddPanel(register_panel_id, name, id);

        id_end = perspective.find_first_of(";|", name_end);
        char buf[16];
        sprintf(buf, "%d", id);
        perspective.replace(name_end + 1, id_end - name_end - 1, buf);
        pos = perspective.find("name=", id_end);
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
        default:
            return;
    }
    wxPanel *panel = AddPanel(type, name);
    if (!output && type == console_panel_id)
        output = (DebugConsole *)panel;
    aui->Update();
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

void DebuggerFrame::CreateMenubar()
{
    wxMenu *filemenu = new wxMenu, *viewmenu = new wxMenu, *statemenu = new wxMenu, *optmenu = new wxMenu;

    filemenu->Append(wxID_CLOSE, _("&Close\tCtrl-Q"));

    run = new wxMenuItem(statemenu, state_run_id, _("Run\tF9"), wxEmptyString, wxITEM_CHECK);
    pause = new wxMenuItem(statemenu, state_pause_id, _("Pause\tF12"), wxEmptyString, wxITEM_CHECK);
    wxMenuItem *separator1 = new wxMenuItem(statemenu);
    wxMenuItem *step = new wxMenuItem(statemenu, state_step_id, _("Step\tF7"));
    wxMenuItem *vibreak = new wxMenuItem(statemenu, state_vibreak_id, _("Next vertical interrupt"));

    run_on_boot_menu = new wxMenuItem(optmenu, run_on_boot_opt_id, _("Run on boot"), wxEmptyString, wxITEM_CHECK);
    runtime_update_menu = new wxMenuItem(optmenu, runtime_update_opt_id, _("Update values while running"), wxEmptyString, wxITEM_CHECK);

    statemenu->Append(run);
    statemenu->Append(pause);
    statemenu->Append(separator1);
    statemenu->Append(step);
    statemenu->Append(vibreak);

    viewmenu->Append(console_panel_id, _("Console"));
    viewmenu->Append(break_panel_id, _("Breakpoints\tCtrl-B"));
    viewmenu->Append(register_panel_id, _("Registers\tCtrl-R"));
    viewmenu->Append(disasm_panel_id, _("Disassembly\tCtrl-D"));
    viewmenu->Append(memory_panel_id, _("Memory\tCtrl-M"));

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
            {
                run_on_boot = 0;
            }
            else
            {
                run_on_boot = 2;
            }
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

void DebuggerFrame::NewBreakpoint(Breakpoint *data)
{

}

void DebuggerFrame::DeleteBreakpoint(Breakpoint *data)
{

}

Breakpoint *DebuggerFrame::FindBreakpoint(int id)
{
    return 0;
}

Breakpoint *DebuggerFrame::FindBreakpoint(wxString name)
{
    return 0;
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

void DebuggerFrame::Print(wxString msg)
{
    if (output)
        output->Print(msg);
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
                pause->Check(true);
                run->Check(false);
                running = false;
                Print(wxString::Format("Paused at %x", evt.GetInt()));
                UpdatePanels();
                Raise();
            }
        }
        break;
        case DEBUG_VI:
        {
            if (vi_break)
                Pause();
            else if (runtime_update)
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
    // TODO--
}

