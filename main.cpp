
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/frame.h>

#include "mupen64plusplus/MupenAPI.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/plugin.h"
#include "parameterpanel.h"
#include <stdexcept>
#include <algorithm>

#include <SDL.h>

// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------

// application class
class wxMiniApp : public wxApp
{
    Mupen64PlusPlus* m_api;
    wxFrame* m_frame;
    wxToolBar* m_toolbar;
    wxStatusBar* m_status_bar;
   
    IConfigurationPanel* m_curr_panel;
    wxBoxSizer* m_sizer;

        
    struct GraphicalSection
    {
        wxToolBarToolBase*         m_tool;
        std::vector<ConfigSection> m_config;
        
        GraphicalSection(wxToolBarToolBase* tool, const ConfigSection& config)
        {
            m_tool = tool;
            m_config.push_back(config);
        }
        
        GraphicalSection(wxToolBarToolBase* tool, const std::vector<ConfigSection>& configs) :
            m_config(configs)
        {
            m_tool = tool;
        }
    };
    
    std::vector<GraphicalSection> m_toolbar_items;


public:
    // function called at the application initialization
    virtual bool OnInit();

    void shutdown()
    {
        if (m_curr_panel != NULL)
        {
            m_curr_panel->commitNewValues();
        }
                
        m_frame->Destroy();

        if (m_api->saveConfig() != M64ERR_SUCCESS)
        {
            wxLogWarning("Failed to save config file");
        }
        delete m_api;        
    }

    void onClose(wxCloseEvent& evt)
    {
        shutdown();
    }
    
    void onQuitMenu(wxCommandEvent& evt)
    {
        shutdown();
    }
    
    // event handler for button click
    void OnClick(wxCommandEvent& event) { GetTopWindow()->Close(); }
    
    void onToolbarItem(wxCommandEvent& evt)
    {
        const int id = evt.GetId();
        std::string section = "[unknown]";
        
        int sectionId = -1;
        
        const int count = m_toolbar_items.size();
        for (int n=0; n<count; n++)
        {
            if (m_toolbar_items[n].m_tool->GetId() == id)
            {
                GraphicalSection& curr = m_toolbar_items[n];
                if (curr.m_config.size() == 1)
                {
                    section = curr.m_config[0].m_section_name;
                }
                else
                {
                    // TODO: find name for group of sections
                    section = curr.m_config[0].m_section_name;
                }
                sectionId = n;
                break;
            }
        }
        
        if (sectionId == -1)
        {
            assert(false);
            return;
        }
        
        // wxMessageBox( "Toolbar item clicked : " + wxString(section.c_str()) );
        
        if (m_curr_panel != NULL)
        {
            m_curr_panel->commitNewValues();
            m_curr_panel->removeMyselfFrom(m_sizer);
            m_curr_panel = NULL;
        }
        
        assert(m_toolbar_items[sectionId].m_config.size() > 0);
        
        if (m_toolbar_items[sectionId].m_config.size() == 1)
        {
            ParameterPanel* newPanel = new ParameterPanel(m_frame, m_toolbar_items[sectionId].m_config[0]);
            newPanel->Layout();
            m_sizer->Add(newPanel, 1, wxEXPAND);
            m_curr_panel = newPanel;
        }
        else
        {
            ParameterGroupsPanel* newPanel = new ParameterGroupsPanel(m_frame, m_toolbar_items[sectionId].m_config);
            newPanel->Layout();
            m_sizer->Add(newPanel, 1, wxEXPAND);
            m_curr_panel = newPanel;
        }
        m_frame->Layout();
        
    }
    
};

IMPLEMENT_APP(wxMiniApp);


bool wxMiniApp::OnInit()
{
    m_curr_panel = NULL;
    
    // SDL_INIT_VIDEO is necessary to init keyboard support, which is in turn necessary for our input module
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    
    printf(" __  __                         __   _  _   ____  _             \n");
    printf("|  \\/  |_   _ _ __   ___ _ __  / /_ | || | |  _ \\| |_   _ ___ \n");
    printf("| |\\/| | | | | '_ \\ / _ \\ '_ \\| '_ \\| || |_| |_) | | | | / __|  \n");
    printf("| |  | | |_| | |_) |  __/ | | | (_) |__   _|  __/| | |_| \\__ \\  \n");
    printf("|_|  |_|\\__,_| .__/ \\___|_| |_|\\___/   |_| |_|   |_|\\__,_|___/  \n");
    printf("             |_|         http://code.google.com/p/mupen64plus/  \n\n");
    
    wxString resources = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
    printf("Will look for resources in <%s>\n", (const char*)resources.utf8_str());
    
    try
    {
        // TODO: on OS X, the default path may need to be updated if the application is moved around
        m_api = new Mupen64PlusPlus(resources + "libmupen64plus" + OSAL_DLL_EXTENSION,
                                    resources.utf8_str(),
                                    "mupen64plus-video-rice",
                                    "mupen64plus-audio-sdl",
                                    "mupen64plus-input-sdl",
                                    "mupen64plus-rsp-hle");
    }
    catch (std::runtime_error& e)
    {
        // TODO: if plugins are not found, offer fixing the paths instead of aborting
        fprintf(stderr, "Sorry, a fatal error was caught :\n%s\n",  e.what());
        return false;
    }
    
    
    m_frame = new wxFrame(NULL, -1, "Mupen64Plus", wxDefaultPosition, wxSize(800, 600));
    
    std::vector<ConfigSection> config;
    
    try
    {
        // FIXME: does not return options from all plugin if mupen64plus --saveoptions was not first invoked
        config = m_api->getConfigContents();
    }
    catch (std::runtime_error& e)
    {
        fprintf(stderr, "Sorry, a fatal error was caught :\n%s\n",  e.what());
        return false;
    }
    
    // -------- Gather configuration sections and options
    
    // For now give an untranslated name to this section, this will allow us to identify it;
    // we'll translate the name later (FIXME: unclean)
    ConfigSection inputSection( "Input", getSectionHandle("Core") ); 
    
    const int configSize = config.size();
    for (int n=0; n<configSize; n++)
    {
        ConfigSection& section = config[n];
        
        printf("==== Section [%s] ====\n", section.m_section_name.c_str());
        
        for (unsigned int p=0; p<section.m_parameters.size(); p++)
        {
            const char* type = "other";
            char buffer[256];
                        switch (section.m_parameters[p].m_param_type)            {                case M64TYPE_INT:                    type = "int";
                    sprintf(buffer, "%i", section.m_parameters[p].getIntValue());                    break;                case M64TYPE_FLOAT:                    type = "float";
                    sprintf(buffer, "%f", section.m_parameters[p].getFloatValue());                    break;                case M64TYPE_BOOL:                    type = "bool";
                    sprintf(buffer, "%i", section.m_parameters[p].getBoolValue());                    break;                case M64TYPE_STRING:                    type = "string";
                    sprintf(buffer, "%s", section.m_parameters[p].getStringValue().c_str());                    break;            }
            
            printf("    - %s %s (%s) = %s\n",
                   type,
                   section.m_parameters[p].m_param_name.c_str(),
                   section.m_parameters[p].m_help_string.c_str(),
                   buffer);
            
            // Move key mappings from wherever they are into a separate input section
            // TODO: find better way to identify key mappings?
            wxString section_wxname(section.m_parameters[p].m_param_name);
            if (section_wxname.StartsWith("Kbd Mapping"))
            {
                section.m_parameters[p].m_special_type = KEYBOARD_KEY_INT;
                inputSection.m_parameters.push_back(section.m_parameters[p]);
                section.m_parameters[p].m_enabled = false;
            }
            else if (section_wxname.StartsWith("Joy Mapping"))
            {
                // TODO: create a special display type for joystick
                //section.m_parameters[p].m_special_type = KEYBOARD_KEY_INT;
                inputSection.m_parameters.push_back(section.m_parameters[p]);
                section.m_parameters[p].m_enabled = false;
            }
        }
    }
    
    config.push_back(inputSection);
    
    
    // -------- Build the toolbar that shows config sections
    m_toolbar = m_frame->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT, wxID_ANY);
    m_toolbar->SetToolBitmapSize(wxSize(32,32));
    
    wxInitAllImageHandlers();
    //wxImage::AddHandler(new wxPNGHandler());
    wxBitmap icon_mupen  (resources + "mupenicon.png", wxBITMAP_TYPE_PNG);    
    wxBitmap icon_input  (resources + "input.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_cpu    (resources + "emulation.png", wxBITMAP_TYPE_PNG);
    wxBitmap icon_audio  (resources + "audio.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_plugins(resources + "plugins.png",   wxBITMAP_TYPE_PNG);
    wxBitmap icon_video  (resources + "video.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_other  (resources + "other.png",     wxBITMAP_TYPE_PNG);
    
    assert(icon_mupen.IsOk());    
    assert(icon_input.IsOk());
    assert(icon_cpu.IsOk());
    assert(icon_audio.IsOk());
    assert(icon_plugins.IsOk());
    assert(icon_video.IsOk());
    assert(icon_other.IsOk());
    
    m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Games"), icon_mupen, icon_mupen),
                                               ConfigSection("Games", (m64p_handle)NULL))); // create a dummy ConfigSection (unused)
    
    std::vector<ConfigSection> inputSections;
    std::vector<ConfigSection> videoSections;
    
    for (unsigned int n=0; n<config.size(); n++)
    {
        ConfigSection& section = config[n];
        
        printf("Adding toolbar button [%s]\n", section.m_section_name.c_str());
        
        // FIXME: find better way than hardcoding sections?
        if (section.m_section_name == "Core")
        {
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Emulation"),
                                                      icon_cpu, icon_cpu), section) );
        }
        else if (section.m_section_name == "UI-Plugins")
        {
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, section.m_section_name,
                                                      icon_plugins, icon_plugins), section) );
        }
        else if (section.m_section_name == "Input")
        {
            section.m_section_name = wxString(_("Shortcut Keys")).mb_str();
            inputSections.push_back(section);
        }
        else if (wxString(section.m_section_name).StartsWith("Video"))
        {
            // strip the "video-" prefix if any
            wxString wxSectionName(section.m_section_name);
            if (wxSectionName.Contains("-"))
            {
                section.m_section_name = (const char*)(wxString(section.m_section_name).AfterFirst('-').mb_str());
            }
            
            videoSections.push_back(section);
            //m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, section.m_section_name,
            //                                           icon_video, icon_video), section));
        }
        else if (section.m_section_name == "Audio-SDL")
        {
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Audio"),
                                                      icon_audio, icon_audio), section));
        }
        else if (wxString(section.m_section_name.c_str()).StartsWith("Input-SDL-Control"))
        {
            wxString idString = wxString(section.m_section_name.c_str()).AfterLast('l');
            long id = -1;
            if (!idString.ToLong(&id))
            {
                wxLogWarning("Cannot parse device ID in " + wxString(section.m_section_name.c_str()));
            }
            
            section.m_section_name = wxString::Format(_("Input Control %i"), (int)id).mb_str();
            
            // add any missing parameter (FIXME: don't hardcode names?) 
            // TODO: add help strings so that they get tooltips
#define CREATE_PARAM_IF_MISSING( name, val, type, disp )                  \
            if (!section.hasChildNamed(name))                             \
                section.addNewParam(name, wxVariant( val ), type, disp);  \
            else                                                          \
                section.getParamWithName(name)->m_special_type = disp
            
            try
            {
                CREATE_PARAM_IF_MISSING("plugged",           true,          M64TYPE_BOOL,   NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("plugin",            1,             M64TYPE_INT,    NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("mouse",             false,         M64TYPE_BOOL,   NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("device",            -2,            M64TYPE_INT,    NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("AnalogDeadzone",    "4096,4096",   M64TYPE_STRING, NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("AnalogPeak",        "32768,32768", M64TYPE_STRING, NOTHING_SPECIAL);
                CREATE_PARAM_IF_MISSING("DPad R",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("DPad L",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("DPad D",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("DPad U",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("Start",             "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("Z Trig",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("B Button",          "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("A Button",          "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("C Button R",        "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("C Button L",        "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("C Button D",        "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("C Button U",        "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("R Trig",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("L Trig",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("Mempak switch",     "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("Rumblepak switch",  "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("X Axis",            "", M64TYPE_STRING, BINDING_STRING);
                CREATE_PARAM_IF_MISSING("Y Axis",            "", M64TYPE_STRING, BINDING_STRING);
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "Error caught while trying to set up %s : %s\n", (const char*)idString.mb_str(), e.what());
            }

#undef CREATE_PARAM_IF_MISSING

            ConfigParam* pluginParam = section.getParamWithName("plugin");
            if (pluginParam != NULL)
            {
                pluginParam->m_choices.push_back( ConfigParamChoice(_("None"), 1) );
                pluginParam->m_choices.push_back( ConfigParamChoice(_("Mem Pack"), 2) );
                pluginParam->m_choices.push_back( ConfigParamChoice(_("Rumble Pack"), 5) );
            }

            ConfigParam* deviceParam = section.getParamWithName("device");
            if (pluginParam != NULL)
            {
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Keyboard/Mouse"), -2) );
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Auto Config"), -1) );
                
                for (int n=0; n<9; n++)
                {
                    deviceParam->m_choices.push_back( ConfigParamChoice(wxString::Format(_("Joystick %i"), n), n) );
                }
            }


            inputSections.push_back(section);
            
            /*
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY,
                                                                               wxString::Format(_("Input Control %i"),
                                                                                                (int)id),
                                                      icon_input, icon_input), section));
            */
        }
        else
        {
            printf("Ignoring config section %s\n", section.m_section_name.c_str());
        }
    }
    
    std::sort(inputSections.begin(), inputSections.end(), ConfigSection::compare);
    m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Input"),
                                              icon_input, icon_input), inputSections));
    m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Video"),
                                              icon_video, icon_video), videoSections));
    
    m_toolbar->Connect(wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(wxMiniApp::onToolbarItem), NULL, this);
    
    m_toolbar->Realize();
    
    m_status_bar = m_frame->CreateStatusBar();
    
    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_frame->SetSizer(m_sizer);
        
    m_frame->Centre();
    m_frame->Show();
    
    m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(wxMiniApp::onClose), NULL, this);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(wxMiniApp::onQuitMenu), NULL, this);
    m_frame->Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(wxMiniApp::onQuitMenu), NULL, this);
    
    SetTopWindow( m_frame );
    
    // enter the application's main loop
    return true;
}
