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
#include <wx/filedlg.h> 

#include "mupen64plusplus/MupenAPI.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/plugin.h"
#include "parameterpanel.h"
#include "gamespanel.h"
#include "sdlkeypicker.h" // to get the USE_WX_KEY_PICKER define
#include "config.h"
#include "main.h"

#include <stdexcept>
#include <algorithm>
#include "sdlhelper.h"

const bool g_Verbose = false;

const char* DEFAULT_VIDEO_PLUGIN = "mupen64plus-video-rice";
const char* DEFAULT_AUDIO_PLUGIN = "mupen64plus-audio-sdl";
const char* DEFAULT_INPUT_PLUGIN = "mupen64plus-input-sdl";
const char* DEFAULT_RSP_PLUGIN   = "mupen64plus-rsp-hle";

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

wxString datadir;
wxString libs;

wxIMPLEMENT_APP_NO_MAIN(MupenFrontendApp);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_RELOAD_OPTIONS);

int main(int argc, char** argv)
{
    MupenFrontendApp* app = new MupenFrontendApp(); 
    wxApp::SetInstance(app);
    return wxEntry(argc, argv);
}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::shutdown()
{
    if (m_curr_panel != NULL)
    {
        m_curr_panel->commitNewValues(true);
    }
    
    if (m_frame != NULL)
    {
        m_frame->Destroy();
        m_frame = NULL;
        m_curr_panel = NULL;
    }

    if (m_api != NULL)
    {
        if (m_api->saveConfig() != M64ERR_SUCCESS)
        {
            wxLogWarning("Failed to save config file");
        }
        delete m_api;
        m_api = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onClose(wxCloseEvent& evt)
{
    shutdown();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onQuitMenu(wxCommandEvent& evt)
{
    shutdown();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::manualRemoveCurrentPanel()
{
    m_curr_panel->removeMyselfFrom(m_sizer);
    m_curr_panel = NULL;
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::manualReshowCurrentPanel()
{
    setCurrentPanel(m_current_panel);
    m_frame->Layout();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::setCurrentPanel(int sectionId)
{
    m_current_panel = sectionId;
    
    try
    {
        if (m_toolbar_items[sectionId].m_is_game_section)
        {
            // games section
            GamesPanel* newPanel = new GamesPanel(m_frame, m_api, m_gamesPathParam);
            newPanel->Layout();
            m_sizer->Add(newPanel, 1, wxEXPAND);
            m_curr_panel = newPanel;
        }
        else if (m_toolbar_items[sectionId].m_config.size() == 1)
        {
            ParameterPanel* newPanel = new ParameterPanel(m_frame, m_api,
                                                          m_toolbar_items[sectionId].m_config[0]);
            newPanel->Layout();
            m_sizer->Add(newPanel, 1, wxEXPAND);
            m_curr_panel = newPanel;
        }
        else
        {
            ParameterGroupsPanel* newPanel = new ParameterGroupsPanel(m_frame, m_api,
                                                                      m_toolbar_items[sectionId].m_config);
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
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onToolbarItem(wxCommandEvent& evt)
{
    const int id = evt.GetId();
    //std::string section = "[unknown]";
    
    // Find which section was clicked
    int sectionId = -1;
    const int count = m_toolbar_items.size();
    for (int n=0; n<count; n++)
    {
        if (m_toolbar_items[n].m_tool->GetId() == id)
        {
            /**
            GraphicalSection& curr = m_toolbar_items[n];
            if (curr.m_is_game_section)
            {
                section = _("Games");
            }
            else if (curr.m_config.size() == 1)
            {
                section = curr.m_config[0]->m_section_name;
            }
            else
            {
                assert(curr.m_config.size() > 1);
                
                // Make a dummy name for the grouped section; not too important since at this
                // point we will most likely not read the name except for debugging purposes
                section = curr.m_config[0]->m_section_name + "-group";
            }
            */
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
        m_curr_panel->commitNewValues(true);
        m_curr_panel->removeMyselfFrom(m_sizer);
        m_curr_panel = NULL;
    }
    
    setCurrentPanel(sectionId);

    m_frame->Layout();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onActivate(wxActivateEvent& evt)
{
#ifdef __WXMAC__
    if (evt.GetActive())
    {
        m_frame->Raise();
    }
#endif
}

// -----------------------------------------------------------------------------------------------------------

bool MupenFrontendApp::OnExceptionInMainLoop()
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

// -----------------------------------------------------------------------------------------------------------

bool MupenFrontendApp::OnInit()
{
    // FIXME: the Glide plugin does not create the config options upon being loaded
    
    // FIXME: the first time mupen opens, a warning that the config file was not found is sent...
    
    m_current_panel  = 0;
    m_curr_panel     = NULL;
    m_gamesPathParam = NULL;
    m_api            = NULL;
    
    printf(" __  __                         __   _  _   ____  _             \n");
    printf("|  \\/  |_   _ _ __   ___ _ __  / /_ | || | |  _ \\| |_   _ ___ \n");
    printf("| |\\/| | | | | '_ \\ / _ \\ '_ \\| '_ \\| || |_| |_) | | | | / __|  \n");
    printf("| |  | | |_| | |_) |  __/ | | | (_) |__   _|  __/| | |_| \\__ \\  \n");
    printf("|_|  |_|\\__,_| .__/ \\___|_| |_|\\___/   |_| |_|   |_|\\__,_|___/  \n");
    printf("             |_|         http://code.google.com/p/mupen64plus/  \n\n");
    
    SDL_Helper_Start();
    
#ifdef DATADIR
    datadir = wxString(DATADIR) + wxFileName::GetPathSeparator();
#else
    datadir = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
#endif

#ifdef LIBDIR
    libs = wxString(LIBDIR) + wxFileName::GetPathSeparator();
#else
    libs = wxStandardPaths::Get().GetPluginsDir() + wxFileName::GetPathSeparator();
#endif

    printf("Will look for resources in <%s> and librairies in <%s>\n", (const char*)datadir.utf8_str(),
                                                                       (const char*)libs.utf8_str());
    int plugins = 0;
    
    wxString corepath = libs + "libmupen64plus" + OSAL_DLL_EXTENSION;
    
    while (m_api == NULL)
    {
        // ---- Init mupen core and plugins
        try
        {
            // TODO: automagically check for local runs (no install) with "OSAL_CURRENT_DIR"?
            m_api = new Mupen64PlusPlus(corepath,
                                        libs.utf8_str(),
                                        DEFAULT_VIDEO_PLUGIN,
                                        DEFAULT_AUDIO_PLUGIN,
                                        DEFAULT_INPUT_PLUGIN,
                                        DEFAULT_RSP_PLUGIN,
                                        (const char*)datadir.utf8_str());
            
            plugins = m_api->loadPlugins();
            if (plugins != 15)
            {
                wxMessageBox( _("Warning, some plugins could not be loaded, please fix the paths before trying to use mupen64plus") );
            }
        }
        catch (CoreNotFoundException& e)
        {
            fprintf(stderr, "The core was not found : %s\n", e.what());
            wxMessageBox( _("The Mupen64Plus core library was not found or loaded; please select it before you can continue") );
            
            wxString wildcard = _("Dynamic libraries") + wxString(" (*") + OSAL_DLL_EXTENSION +
                                      ")|*" + OSAL_DLL_EXTENSION + "|" + _("All files") + "|*";
            
            corepath = wxFileSelector(_("Select the Mupen64Plus core library"),
                                      libs, wxEmptyString, OSAL_DLL_EXTENSION, 
                                      wildcard, wxFD_OPEN);
            
            // Quit if user cancelled
            if (corepath.IsEmpty()) return false;
        }
        catch (std::runtime_error& e)
        {
            fprintf(stderr, "Sorry, a fatal error was caught :\n%s\n",  e.what());
            wxMessageBox( _("Sorry, initializing Mupen64Plus failed. Please verify the integrity of your installation.") );
            return false;
        }
    } // end while
    
    m_frame = new wxFrame(NULL, -1, "Mupen64Plus", wxDefaultPosition, wxSize(1024, 640));
    
    wxInitAllImageHandlers();
    if (not makeToolbar(plugins, 0)) return false;
    
    m_status_bar = m_frame->CreateStatusBar();
    
    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    GamesPanel* games = new GamesPanel(m_frame, m_api, m_gamesPathParam);
    m_sizer->Add(games, 1, wxEXPAND);
    m_curr_panel = games;
    
    wxMenuBar* bar = new wxMenuBar();
    
    wxMenu* file = new wxMenu();
    //wxApp::s_macExitMenuItemId = wxID_EXIT;
    file->Append(wxID_EXIT, _("&Quit") + "\tCtrl-Q");
    file->Append(wxID_ABOUT, _("&About"));
    
    bar->Append(file, _("File"));
    m_frame->SetMenuBar(bar);
    
    m_frame->SetSizer(m_sizer);
    
    m_frame->Centre();
    m_frame->Show();
    
    m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MupenFrontendApp::onClose), NULL, this);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    m_frame->Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
                     wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    
    SetTopWindow( m_frame );
    Connect(wxID_ANY, wxEVT_ACTIVATE_APP, wxActivateEventHandler(MupenFrontendApp::onActivate), NULL, this);
    
    Connect(wxID_ANY, wxMUPEN_RELOAD_OPTIONS, wxCommandEventHandler(MupenFrontendApp::onReloadOptionsRequest), NULL, this);
    
    // enter the application's main loop
    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool MupenFrontendApp::makeToolbar(int plugins, int selectedSection)
{
    wxToolBar* tb = m_frame->GetToolBar();
    m_frame->SetToolBar(NULL);
    if (tb != NULL) delete tb;
    m_toolbar_items.clear();
    
    m_config.clearAndDeleteAll();
    
    // ---- Get config options
    try
    {
        getOptions( m_api, &m_config );
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
    wxToolBarToolBase* t = m_toolbar->AddRadioTool(wxID_ANY, _("Games"), icon_mupen, icon_mupen);
    
    m_toolbar_items.push_back(GraphicalSection::createGamesSection(t));
    
    std::vector<ConfigSection*> inputSections;
    std::vector<ConfigSection*> videoSections;
    
    const unsigned count = m_config.size();
    for (unsigned int n=0; n<count; n++)
    {
        ConfigSection* section = m_config[n];
                
        // FIXME: find better way than hardcoding sections?
        if (section->m_section_name == "Core")
        {
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Emulation"),
                                                      icon_cpu, icon_cpu), section) );
        }
        else if (section->m_section_name == "UI-wx")
        {
            ConfigParam* ptr_param = section->getParamWithName("GamesPath");
            if (ptr_param != NULL)
            {
                ptr_param->m_enabled = false;
                m_gamesPathParam = ptr_param;
            }
            
            ConfigParam* pluginsDir = section->getParamWithName("PluginDir");
            if (pluginsDir != NULL)
            {
                ConfigParam* plugin1 = section->getParamWithName("VideoPlugin");
                if (plugin1 != NULL)
                {
                    plugin1->m_special_type = PLUGIN_FILE;
                    plugin1->m_is_ok = (plugins & 0x1) != 0;
                    plugin1->m_dir = new ConfigParam(*pluginsDir);
                }
                
                ConfigParam* plugin2 = section->getParamWithName("AudioPlugin");
                if (plugin2 != NULL)
                {
                    plugin2->m_special_type = PLUGIN_FILE;
                    plugin2->m_is_ok = (plugins & 0x2) != 0;
                    plugin2->m_dir = new ConfigParam(*pluginsDir);
                }
                
                ConfigParam* plugin3 = section->getParamWithName("InputPlugin");
                if (plugin3 != NULL)
                {
                    plugin3->m_special_type = PLUGIN_FILE;
                    plugin3->m_is_ok = (plugins & 0x4) != 0;
                    plugin3->m_dir = new ConfigParam(*pluginsDir);
                }
                
                ConfigParam* plugin4 = section->getParamWithName("RspPlugin");
                if (plugin4 != NULL)
                {
                    plugin4->m_special_type = PLUGIN_FILE;
                    plugin4->m_is_ok = (plugins & 0x8) != 0;
                    plugin4->m_dir = new ConfigParam(*pluginsDir);
                }
            }
            else
            {
                wxLogError("Cannot find the plugins path parameter!");
            }
            
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Plugins"),
                                                      icon_plugins, icon_plugins), section) );
        }
        else if (section->m_section_name == "Input")
        {
            section->m_section_name = wxString(_("Shortcut Keys")).mb_str();
            inputSections.push_back(section);
        }
        else if (wxString(section->m_section_name).StartsWith("Video"))
        {
            // strip the "video-" prefix if any
            wxString wxSectionName(section->m_section_name);
            if (wxSectionName.Contains("-"))
            {
                section->m_section_name = (const char*)(
                        wxString(section->m_section_name).AfterFirst('-').mb_str()
                    );
            }
            
            videoSections.push_back(section);
        }
        else if (section->m_section_name == "Audio-SDL")
        {                
            m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Audio"),
                                                      icon_audio, icon_audio), section));
        }
        else if (wxString(section->m_section_name.c_str()).StartsWith("Input-SDL-Control"))
        {
            wxString idString = wxString(section->m_section_name.c_str()).AfterLast('l');
            long id = -1;
            if (!idString.ToLong(&id))
            {
                wxLogWarning("Cannot parse device ID in " + wxString(section->m_section_name.c_str()));
            }
            
            section->m_section_name = wxString::Format(_("Input Control %i"), (int)id).mb_str();
            
            inputSections.push_back(section);
        }
        else
        {
            printf("Ignoring config section %s\n", section->m_section_name.c_str());
        }
    }
    
    std::sort(inputSections.begin(), inputSections.end(), ConfigSection::compare);
    m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Input"),
                                              icon_input, icon_input), inputSections));
    m_toolbar_items.push_back(GraphicalSection(m_toolbar->AddRadioTool(wxID_ANY, _("Video"),
                                              icon_video, icon_video), videoSections));
    
    m_toolbar->Connect(wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MupenFrontendApp::onToolbarItem),
                       NULL, this);
    
    m_toolbar->Realize();
    
    const int amount = m_toolbar_items.size();
    if (selectedSection >= amount or selectedSection < 0) selectedSection = 0;
    
    m_toolbar->ToggleTool( m_toolbar_items[selectedSection].m_tool->GetId(), true );
    
    return true;
}


void MupenFrontendApp::onReloadOptionsRequest(wxCommandEvent& evt)
{
    // On wxOSX, changing the toolbar has the ugly disadvantage of resizing the frame...
    m_frame->Freeze();
    wxSize size = m_frame->GetSize();
    
    manualRemoveCurrentPanel();
    makeToolbar(evt.GetInt(), m_current_panel);
    manualReshowCurrentPanel();
    
    m_frame->SetSize(size);
    m_frame->Thaw();
}
