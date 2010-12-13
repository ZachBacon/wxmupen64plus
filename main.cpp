/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus frontend                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marianne Gagnon, based on work by Richard Goedeken *
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
 
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/frame.h>
#include <wx/event.h>

#include "mupen64plusplus/MupenAPI.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/plugin.h"
#include "parameterpanel.h"
#include "gamespanel.h"
#include "sdlkeypicker.h" // to get the USE_WX_KEY_PICKER define

#include <stdexcept>
#include <algorithm>

#include <SDL.h>

const bool g_Verbose = false;

extern "C"
{
    void DebugCallback(void *Context, int level, const char *message)
    {
        if (level <= 1)
        {
            printf("%s Error: %s\n", (const char *) Context, message);
            wxLogError( _("[%s] An error occurred : %s"), (const char *) Context, message );
        }
        else if (level == 2)
        {
            printf("%s Warning: %s\n", (const char *) Context, message);
            wxLogWarning( _("[%s] Warning : %s"), (const char *) Context, message );
        }
        else if (level == 3 || (level == 5 && g_Verbose))
        {
            printf("%s: %s\n", (const char *) Context, message);
        }
        else if (level == 4)
        {
            printf("%s Status: %s\n", (const char *) Context, message);
        }
        /* ignore the verbose info for now */
    }
}

// --------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------

// application class
class MupenFrontendApp : public wxApp
{
    Mupen64PlusPlus* m_api;
    wxFrame* m_frame;
    wxToolBar* m_toolbar;
    wxStatusBar* m_status_bar;
   
    IConfigurationPanel* m_curr_panel;
    wxBoxSizer* m_sizer;

    ConfigParam m_gamesPathParam;
    
    /**
     * Associates a configuration section with its toolbar icon
     * (FIXME: rename this class, what it does is not obvious)
     */
    struct GraphicalSection
    {
        /** The associated toolbar button */
        wxToolBarToolBase*         m_tool;
        
        /** 
         * The mupen API config section associated with this button (some buttons may contain more than
         * one config section)
         */
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
    
    /** List of configuration sections */
    std::vector<GraphicalSection> m_toolbar_items;

public:
    virtual bool OnInit();

    std::vector<ConfigSection> getOptions();

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
    
    /**
     * Callback invoked when a toolbar item is clicked.
     * The general action to perform when this happens is to change the currently displayed pane
     */
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
                    // Make a dummy name for the grouped section; not too important since at this
                    // point we will most likely not read the name except for debugging purposes
                    section = curr.m_config[0].m_section_name + "-group";
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
        
        if (m_curr_panel != NULL)
        {
            m_curr_panel->commitNewValues();
            m_curr_panel->removeMyselfFrom(m_sizer);
            m_curr_panel = NULL;
        }
        
        assert(m_toolbar_items[sectionId].m_config.size() > 0);
        
        try
        {
            if (sectionId == 0) // FIXME: don't rely that the "games" section is first?
            {
                // games section
                GamesPanel* newPanel = new GamesPanel(m_frame, m_api, m_gamesPathParam);
                newPanel->Layout();
                m_sizer->Add(newPanel, 1, wxEXPAND);
                m_curr_panel = newPanel;
            }
            else if (m_toolbar_items[sectionId].m_config.size() == 1)
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
        }
        catch (std::runtime_error& e)
        {
            wxMessageBox( wxString("Sorry, an error occurred : ") + e.what() );
            m_curr_panel = NULL;
        }

        m_frame->Layout();
        
    }
    
    void onActivate(wxActivateEvent& evt)
    {
    #ifdef __WXMAC__
        if (evt.GetActive())
        {
            m_frame->Raise();
        }
    #endif
    }
    
    virtual bool OnExceptionInMainLoop()
    {
        wxString what = "Unknown error";
        
        try
        {
            throw;
        }
        catch (std::exception& e)
        {
            what = e.what();
        }
        
        std::cerr << "/!\\ An internal error occurred : an exception was caught unhandled\n" << what.mb_str()
                  << std::endl;
        wxMessageBox(_("Sorry an internal error occurred : an exception was caught unhandled : ") + what);
        return true;
    }
};

IMPLEMENT_APP(MupenFrontendApp);

// -----------------------------------------------------------------------------------------------------------

bool MupenFrontendApp::OnInit()
{
    // FIXME: the first time a video plugin is selected, it does not appear in optioms =(
    // (at least with Glide, I needed to first launch a game with it, then restart the frontend)
    
    m_curr_panel = NULL;
    m_gamesPathParam = NULL;
    
    #if USE_WX_KEY_PICKER
    // SDL_INIT_VIDEO is necessary to init keyboard support, which is in turn necessary for our input module
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    #endif
    
    printf(" __  __                         __   _  _   ____  _             \n");
    printf("|  \\/  |_   _ _ __   ___ _ __  / /_ | || | |  _ \\| |_   _ ___ \n");
    printf("| |\\/| | | | | '_ \\ / _ \\ '_ \\| '_ \\| || |_| |_) | | | | / __|  \n");
    printf("| |  | | |_| | |_) |  __/ | | | (_) |__   _|  __/| | |_| \\__ \\  \n");
    printf("|_|  |_|\\__,_| .__/ \\___|_| |_|\\___/   |_| |_|   |_|\\__,_|___/  \n");
    printf("             |_|         http://code.google.com/p/mupen64plus/  \n\n");
    
    
    #if USE_WX_KEY_PICKER
    // ====================
    // Init Gamepad Support
    printf("%i joysticks were found.\n\n", SDL_NumJoysticks() );
    printf("The names of the joysticks are:\n");
    SDL_JoystickEventState(SDL_ENABLE);
    
    for (int i=0; i<SDL_NumJoysticks(); i++) 
    {
        printf("    %s\n", SDL_JoystickName(i));
        /* SDL_Joystick *joystick = */ SDL_JoystickOpen(i); // TODO: also close them on shutdown
    }
    printf("\n");
    // ====================
    #endif
    
#ifdef DATADIR
    wxString datadir = wxString(DATADIR) + wxFileName::GetPathSeparator();
#else
    wxString datadir = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
#endif

#ifdef LIBDIR
    wxString libs = wxString(LIBDIR) + wxFileName::GetPathSeparator();
#else
    wxString libs = wxStandardPaths::Get().GetPluginsDir() + wxFileName::GetPathSeparator();
#endif

    printf("Will look for resources in <%s> and librairies in <%s>\n", (const char*)datadir.utf8_str(),
                                                                       (const char*)libs.utf8_str());
    
    // ---- Init mupen core and plugins
    try
    {
        // TODO: automagically check for local runs (no install) with "OSAL_CURRENT_DIR"?
        m_api = new Mupen64PlusPlus(libs + "libmupen64plus" + OSAL_DLL_EXTENSION,
                                    libs.utf8_str(),
                                    "mupen64plus-video-rice",
                                    "mupen64plus-audio-sdl",
                                    "mupen64plus-input-sdl",
                                    "mupen64plus-rsp-hle");
    }
    catch (std::runtime_error& e)
    {
        // TODO: if plugins are not found, offer fixing the paths instead of aborting
        fprintf(stderr, "Sorry, a fatal error was caught :\n%s\n",  e.what());
        wxMessageBox( wxString::Format( _("Sorry, a fatal error was caught :\n%s"),  e.what() ) );
        return false;
    }
    
    
    m_frame = new wxFrame(NULL, -1, "Mupen64Plus", wxDefaultPosition, wxSize(1024, 640));
    
    // ---- Get config options
    std::vector<ConfigSection> config;
    try
    {
        config = getOptions();
    }
    catch (std::runtime_error& e)
    {
        fprintf(stderr, "Sorry, a fatal error was caught :\n%s\n",  e.what());
        wxMessageBox( wxString::Format( _("Sorry, a fatal error was caught :\n%s"),  e.what() ) );
        return false;
    }
    
    // ---- Build the toolbar that shows config sections
    m_toolbar = m_frame->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT, wxID_ANY);
    m_toolbar->SetToolBitmapSize(wxSize(32,32));
    
    wxInitAllImageHandlers();
    wxBitmap icon_mupen  (datadir + "mupenicon.png", wxBITMAP_TYPE_PNG);    
    wxBitmap icon_input  (datadir + "input.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_cpu    (datadir + "emulation.png", wxBITMAP_TYPE_PNG);
    wxBitmap icon_audio  (datadir + "audio.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_plugins(datadir + "plugins.png",   wxBITMAP_TYPE_PNG);
    wxBitmap icon_video  (datadir + "video.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_other  (datadir + "other.png",     wxBITMAP_TYPE_PNG);
    
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
                
        // FIXME: find better way than hardcoding sections?
        if (section.m_section_name == "Core")
        {
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Emulation"),
                                                      icon_cpu, icon_cpu), section) );
        }
        else if (section.m_section_name == "UI-wx")
        {
            ConfigParam* ptr_param = section.getParamWithName("GamesPath");
            if (ptr_param != NULL)
            {
                ptr_param->m_enabled = false;
                m_gamesPathParam = *ptr_param;
			}
			
			ConfigParam* pluginsDir = section.getParamWithName("PluginDir");
            if (pluginsDir != NULL)
			{
				ConfigParam* plugin1 = section.getParamWithName("VideoPlugin");
				if (plugin1 != NULL)
				{
					plugin1->m_special_type = PLUGIN_FILE;
					plugin1->m_dir = new ConfigParam(*pluginsDir);
				}
				
				ConfigParam* plugin2 = section.getParamWithName("AudioPlugin");
				if (plugin2 != NULL)
				{
					plugin2->m_special_type = PLUGIN_FILE;
					plugin2->m_dir = new ConfigParam(*pluginsDir);
				}
				
				ConfigParam* plugin3 = section.getParamWithName("InputPlugin");
				if (plugin3 != NULL)
				{
					plugin3->m_special_type = PLUGIN_FILE;
					plugin3->m_dir = new ConfigParam(*pluginsDir);
				}
				
				ConfigParam* plugin4 = section.getParamWithName("RspPlugin");
				if (plugin4 != NULL)
				{
					plugin4->m_special_type = PLUGIN_FILE;
					plugin4->m_dir = new ConfigParam(*pluginsDir);
				}
            }
			else
			{
				wxLogError("Cannot find the plugins path parameter!");
			}
			
            // TODO: when a plugin is changed, load it, and get its config options
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Plugins"),
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
            
            inputSections.push_back(section);
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
    
    m_toolbar->Connect(wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MupenFrontendApp::onToolbarItem), NULL, this);
    
    m_toolbar->Realize();
    m_toolbar->ToggleTool( m_toolbar_items[0].m_tool->GetId(), true );
    
    m_status_bar = m_frame->CreateStatusBar();
    
    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    GamesPanel* games = new GamesPanel(m_frame, m_api, m_gamesPathParam);
    m_sizer->Add(games, 1, wxEXPAND);
    m_curr_panel = games;
    
    m_frame->SetSizer(m_sizer);
        
    m_frame->Centre();
    m_frame->Show();
    
    m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MupenFrontendApp::onClose), NULL, this);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    m_frame->Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    
    SetTopWindow( m_frame );
    Connect(wxID_ANY, wxEVT_ACTIVATE_APP, wxActivateEventHandler(MupenFrontendApp::onActivate), NULL, this);
    
    // enter the application's main loop
    return true;
}

// -----------------------------------------------------------------------------------------------------------

#define CHATTY 0

std::vector<ConfigSection> MupenFrontendApp::getOptions()
{
    // FIXME: this entire function could be improved; at this point, the structure of the config is
    //        hardcoded. Ideally this would be loaded ffrom some config file (but even more ideally
    //        the mupen core would provide me with this information)
    
    std::vector<ConfigSection> config = m_api->getConfigContents();
    
    // For now give an untranslated name to this section, this will allow us to identify it;
    // we'll translate the name later (FIXME: unclean)
    ConfigSection inputSection( "Input", getSectionHandle("Core") ); 
    
    const int configSize = config.size();
    for (int n=0; n<configSize; n++)
    {
        ConfigSection& section = config[n];
        
#if CHATTY
        printf("==== Section [%s] ====\n", section.m_section_name.c_str());
#endif

        for (unsigned int p=0; p<section.m_parameters.size(); p++)
        {
#if CHATTY
            const char* type = "other";
            char buffer[256];

            switch (section.m_parameters[p].m_param_type)
            {
                case M64TYPE_INT:
                    type = "int";
                    sprintf(buffer, "%i", section.m_parameters[p].getIntValue());
                    break;
                case M64TYPE_FLOAT:
                    type = "float";
                    sprintf(buffer, "%f", section.m_parameters[p].getFloatValue());
                    break;
                case M64TYPE_BOOL:
                    type = "bool";
                    sprintf(buffer, "%i", section.m_parameters[p].getBoolValue());
                    break;
                case M64TYPE_STRING:
                    type = "string";
                    sprintf(buffer, "%s", section.m_parameters[p].getStringValue().c_str());
                    break;
            }
            
            printf("    - %s %s (%s) = %s\n",
                   type,
                   section.m_parameters[p].m_param_name.c_str(),
                   section.m_parameters[p].m_help_string.c_str(),
                   buffer);
#endif

            wxString param_wxname(section.m_parameters[p].m_param_name);
            
            // Move key mappings from wherever they are into a separate input section
            if (param_wxname.StartsWith("Kbd Mapping"))
            {
                section.m_parameters[p].m_special_type = KEYBOARD_KEY_INT;
                inputSection.m_parameters.push_back(section.m_parameters[p]);
                section.m_parameters[p].m_enabled = false;
            }
            else if (param_wxname.StartsWith("Joy Mapping"))
            {
                section.m_parameters[p].m_special_type = BINDING_DIGITAL_STRING;
                inputSection.m_parameters.push_back(section.m_parameters[p]);
                section.m_parameters[p].m_enabled = false;
            }
            else if (param_wxname == "PluginDir" || param_wxname == "ScreenshotPath" ||
                     param_wxname == "SaveStatePath" || param_wxname == "SharedDataPath")
            {
                section.m_parameters[p].m_special_type = DIRECTORY;
            }
            else if (param_wxname == "R4300Emulator")
            {
                ConfigParam& param = section.m_parameters[p];
                param.m_choices.push_back( ConfigParamChoice(_("Pure Interpreter"), 0) );
                param.m_choices.push_back( ConfigParamChoice(_("Cached Interpreter"), 1) );
                param.m_choices.push_back( ConfigParamChoice(_("Dynamic Recompiler"), 2) );
            }
        } // end for each parameter
        
        if (wxString(section.m_section_name).StartsWith("Video"))
        {
            if (wxString(section.m_section_name).Contains("Rice"))
            {
                ConfigParam* frameBufferSetting = section.getParamWithName("FrameBufferSetting");
                if (frameBufferSetting != NULL)
                {
                    frameBufferSetting->m_choices.push_back( ConfigParamChoice(_("ROM Default"), 0) );
                    frameBufferSetting->m_choices.push_back( ConfigParamChoice(_("Disabled"), 1) );
                }
                
                ConfigParam* rttSetting = section.getParamWithName("RenderToTexture");
                if (rttSetting != NULL)
                {
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("None"), 0) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Ignore"), 1) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Normal"), 2) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Write Back"), 3) );
                    rttSetting->m_choices.push_back( ConfigParamChoice(_("Write Back and Reload"), 4) );
                }
                
                ConfigParam* screenUpSetting = section.getParamWithName("ScreenUpdateSetting");
                if (screenUpSetting != NULL)
                {
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("ROM Default"), 0) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("VI Origin Update"), 1) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("VI Origin Change"), 2) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("CI Change"), 3) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("First CI Change"), 4) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("First Primitive Drawn"), 5) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("Before Screen Clear"), 6) );
                    screenUpSetting->m_choices.push_back( ConfigParamChoice(_("After Screen Clear"), 7) );
                }
                
                ConfigParam* fogSetting = section.getParamWithName("FogMethod");
                if (fogSetting != NULL)
                {
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Disable"), 0) );
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Enable N64 Choose"), 1) );
                    fogSetting->m_choices.push_back( ConfigParamChoice(_("Force Fog"), 2) );                    
                }
                
                ConfigParam* forceTextureFilterSetting = section.getParamWithName("ForceTextureFilter");
                if (forceTextureFilterSetting != NULL)
                {
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Auto (N64 Choose)"), 0) );
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Force no Filtering"), 1) );
                    forceTextureFilterSetting->m_choices.push_back( ConfigParamChoice(_("Force Filtering"), 2) );                    
                }
                
                ConfigParam* textureFilteringMethod = section.getParamWithName("TextureFilteringMethod");
                if (textureFilteringMethod != NULL)
                {
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("No Filtering"), 0) );
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("Bilinear"), 1) );
                    textureFilteringMethod->m_choices.push_back( ConfigParamChoice(_("Trilinear"), 2) );                    
                }
                
                ConfigParam* textureEnhancement = section.getParamWithName("TextureEnhancement");
                if (textureEnhancement != NULL)
                {
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("None"), 0) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("2X"), 1) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("2XSAI"), 2) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("HQ2X"), 3) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("LQ2X"), 4) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("HQ4X"), 5) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Sharpen"), 6) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Sharpen More"), 7) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("External"), 8) );
                    textureEnhancement->m_choices.push_back( ConfigParamChoice(_("Mirror"), 9) );
                }
                
                ConfigParam* textureQuality = section.getParamWithName("TextureQuality");
                if (textureQuality != NULL)
                {
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("Defaut"), 0) );
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 1) );
                    textureQuality->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 2) );
                }
                
                ConfigParam* multiSampling = section.getParamWithName("MultiSampling");
                if (multiSampling != NULL)
                {
                    multiSampling->m_choices.push_back( ConfigParamChoice(_("Off"), 0) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("2", 2) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("4", 4) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("8", 8) );
                    multiSampling->m_choices.push_back( ConfigParamChoice("16", 16) );
                }
                
                ConfigParam* colorQuality = section.getParamWithName("ColorQuality");
                if (colorQuality != NULL)
                {
                    colorQuality->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 0) );
                    colorQuality->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 1) );
                }
                
                ConfigParam* openGLDepthBufferSetting = section.getParamWithName("OpenGLDepthBufferSetting");
                if (openGLDepthBufferSetting != NULL)
                {
                    openGLDepthBufferSetting->m_choices.push_back( ConfigParamChoice(_("32 Bits"), 32) );
                    openGLDepthBufferSetting->m_choices.push_back( ConfigParamChoice(_("16 Bits"), 16) );
                }
                
                ConfigParam* openGLRenderSetting = section.getParamWithName("OpenGLRenderSetting");
                if (openGLRenderSetting != NULL)
                {
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice(_("Auto"), 0) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.1", 1) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.2", 2) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.3", 3) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.4", 4) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL 1.4 v2", 5) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("OGL TNT2", 6) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice("NVIDIA OGL", 7) );
                    openGLRenderSetting->m_choices.push_back( ConfigParamChoice(_("OGL Fragment Program"), 8) );
                }
            }
        }
        else if (section.m_section_name == "Audio-SDL")
        {
            ConfigParam* resample = section.getParamWithName("RESAMPLE");
            if (resample != NULL)
            {
                resample->m_choices.push_back( ConfigParamChoice(_("Unfiltered"), 1) );
                resample->m_choices.push_back( ConfigParamChoice(_("SINC Resampling"), 2) );
            }
            
            ConfigParam* volumeCtrType = section.getParamWithName("VOLUME_CONTROL_TYPE");
            if (volumeCtrType != NULL)
            {
                volumeCtrType->m_choices.push_back( ConfigParamChoice(_("SDL (mupen output only)"), 1) );
                volumeCtrType->m_choices.push_back( ConfigParamChoice(_("OSS mixer (master PC volume)"), 2) );
            }
        }
        else if (wxString(section.m_section_name.c_str()).StartsWith("Input-SDL-Control"))
        {
            // Add any missing parameter
#define CREATE_PARAM_IF_MISSING( name, val, type, disp, help )                  \
            if (!section.hasChildNamed(name))                                   \
                section.addNewParam(name, help, wxVariant( val ), type, disp);  \
            else                                                                \
                section.getParamWithName(name)->m_special_type = disp
            
            try
            {
                CREATE_PARAM_IF_MISSING("plugged",           false,         M64TYPE_BOOL,   NOTHING_SPECIAL, "Specifies whether this input device is currently enabled");
                CREATE_PARAM_IF_MISSING("plugin",            1,             M64TYPE_INT,    NOTHING_SPECIAL, "Expansion pack type, if any");
                CREATE_PARAM_IF_MISSING("mouse",             false,         M64TYPE_BOOL,   NOTHING_SPECIAL, "Whether mouse use is enabled");
                CREATE_PARAM_IF_MISSING("device",            -2,            M64TYPE_INT,    NOTHING_SPECIAL, "Specifies which input device to use");
                
                // TODO: provide nicer way to edit deadzone and peak
                CREATE_PARAM_IF_MISSING("AnalogDeadzone",    "4096,4096",   M64TYPE_STRING, NOTHING_SPECIAL, "For analog controls, specifies the dead zone");
                CREATE_PARAM_IF_MISSING("AnalogPeak",        "32768,32768", M64TYPE_STRING, NOTHING_SPECIAL, "For analog controls, specifies the peak value");
                CREATE_PARAM_IF_MISSING("DPad R",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Right button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad L",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Left button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad D",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Down button on the digital pad");
                CREATE_PARAM_IF_MISSING("DPad U",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "Up button on the digital pad");
                CREATE_PARAM_IF_MISSING("Start",             "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Z Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("B Button",          "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("A Button",          "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("C Button R",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Right button");
                CREATE_PARAM_IF_MISSING("C Button L",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Left button");
                CREATE_PARAM_IF_MISSING("C Button D",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Down button");
                CREATE_PARAM_IF_MISSING("C Button U",        "", M64TYPE_STRING, BINDING_DIGITAL_STRING, "C-Up button");
                CREATE_PARAM_IF_MISSING("R Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("L Trig",            "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Mempak switch",     "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("Rumblepak switch",  "", M64TYPE_STRING, BINDING_DIGITAL_STRING, NULL);
                CREATE_PARAM_IF_MISSING("X Axis",            "", M64TYPE_STRING, BINDING_ANALOG_COUPLE_STRING, "Horizontal analog axis");
                CREATE_PARAM_IF_MISSING("Y Axis",            "", M64TYPE_STRING, BINDING_ANALOG_COUPLE_STRING, "Vertical analog axis");
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "Error caught while trying to set up %s : %s\n", (const char*)section.m_section_name.c_str(), e.what());
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
            if (deviceParam != NULL)
            {
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Keyboard/Mouse"), -2) );
                deviceParam->m_choices.push_back( ConfigParamChoice(_("Auto Config"), -1) );
                
                for (int n=0; n<9; n++)
                {
                    deviceParam->m_choices.push_back( ConfigParamChoice(wxString::Format(_("Joystick %i"), n), n) );
                }
            }
        }
        
    } // end for each section
    
    config.push_back(inputSection);
    return config;
}

// -----------------------------------------------------------------------------------------------------------
