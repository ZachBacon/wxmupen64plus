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
#include <wx/html/htmlwin.h>
#include <wx/progdlg.h>
#include <wx/artprov.h>
#include <SDL.h>

#include "mupen64plusplus/MupenAPI.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/plugin.h"
#include "parameterpanel.h"
#include "gamespanel.h"
#include "sdlkeypicker.h" // to get the USE_WX_KEY_PICKER define
#include "config.h"
#include "main.h"
#include "wxvidext.h"


#ifdef ENABLE_DEBUGGER
#include "debugger/debuggerframe.h"
#endif


#include <stdexcept>
#include <algorithm>
#include "sdlhelper.h"

#ifdef __WXGTK__
#include <Xlib.h>
//int XInitThreads();
#endif

const bool g_Verbose = false;

const int VERSION_MAJOR = 0;
const int VERSION_MINOR = 3;

const char* DEFAULT_VIDEO_PLUGIN = "mupen64plus-video-rice";
const char* DEFAULT_AUDIO_PLUGIN = "mupen64plus-audio-sdl";
const char* DEFAULT_INPUT_PLUGIN = "mupen64plus-input-sdl";
const char* DEFAULT_RSP_PLUGIN   = "mupen64plus-rsp-hle";

bool osd_logging = false;

void setOsdLogging(bool p)
{
    osd_logging = p;
}

extern "C"
{
    void DebugCallback(void *Context, int level, const char *message)
    {
        if (osd_logging)
        {
            // TODO: if someday OSD is exposed to the frontend, use it
            if (level <= 1)
            {
                wxString msg = wxString::Format( "%s Error : %s", (const char*)Context, message );
                printf("%s\n", (const char*)msg.utf8_str());
                //osdNewMessage(msg.utf8_str());
            }
            else if (level == 2)
            {
                wxString msg = wxString::Format( "%s Warning : %s", (const char*)Context, message );
                printf("%s\n", (const char*)msg.utf8_str());
                //osdNewMessage(msg.utf8_str());
            }
        }
        else
        {
            if (level <= 1)
            {
                mplog_error("Core", "%s Error: %s\n", (const char *) Context, message);
                wxLogError( _("[%s] An error occurred : %s"), (const char *) Context, message );
            }
            else if (level == 2)
            {
                mplog_warning("Core", "%s Warning: %s\n", (const char *) Context, message);
                wxLogWarning( _("[%s] Warning : %s"), (const char *) Context, message );
            }
            else if (level == 3 || (level == 5 && g_Verbose))
            {
                mplog_info("Core", "%s: %s\n", (const char *) Context, message);
            }
            else if (level == 4)
            {
                mplog_info("Core", "%s Status: %s\n", (const char *) Context, message);
            }
            /* ignore the verbose info for now */
        }
    }
}

wxString datadir;
wxString libs;

wxIMPLEMENT_APP_NO_MAIN(MupenFrontendApp);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_RELOAD_OPTIONS);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_READ_OPEN_FILE_QUEUE);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_INIT_GL_CANVAS);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_INITED_GL_CANVAS);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_CLEAN_GL_CANVAS);

int main(int argc, char** argv)
{
    wxDISABLE_DEBUG_SUPPORT();
    
#ifdef __WXGTK__
    XInitThreads();
#endif

    MupenFrontendApp* app = new MupenFrontendApp(); 
    wxApp::SetInstance(app);
    return wxEntry(argc, argv);
}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

bool MupenFrontendApp::OnInit()
{
    m_inited = false;
    
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
    
#ifdef WXDATADIR
    datadir = wxString(WXDATADIR) + wxFileName::GetPathSeparator();
#else
    datadir = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
#endif

#ifdef LIBDIR
    libs = wxString(LIBDIR) + wxFileName::GetPathSeparator();
#else
    libs = wxStandardPaths::Get().GetPluginsDir() + wxFileName::GetPathSeparator();
#endif

    wxString pluginsPath = libs;
#ifdef PLUGINSDIR
    pluginsPath = wxString(PLUGINSDIR) + wxFileName::GetPathSeparator();
#endif

    mplog_info("MupenfrontApp", "Will look for resources in <%s> and librairies in <%s>\n",
              (const char*)datadir.utf8_str(), (const char*)libs.utf8_str());
    int plugins = 0;
    
    wxString corepath = libs + OSAL_DEFAULT_DYNLIB_FILENAME; //"libmupen64plus" + OSAL_DLL_EXTENSION;
    
    while (m_api == NULL)
    {
        // ---- Init mupen core and plugins
        try
        {
            // TODO: automagically check for local runs (no install) with "OSAL_CURRENT_DIR"?
            m_api = new Mupen64PlusPlus(toNativeEncoding(corepath),
                                        toNativeEncoding(pluginsPath),
                                        DEFAULT_VIDEO_PLUGIN,
                                        DEFAULT_AUDIO_PLUGIN,
                                        DEFAULT_INPUT_PLUGIN,
                                        DEFAULT_RSP_PLUGIN,
                                        toNativeEncoding(datadir));
            
            plugins = m_api->loadPlugins();
            if (plugins != 15)
            {
                wxMessageBox( _("Warning, some plugins could not be loaded, please fix the paths before trying to use mupen64plus") );
            }
        }
        catch (CoreNotFoundException& e)
        {
            mplog_error("MupenAPI", "The core was not found : %s\n", e.what());
            wxMessageBox( wxString::Format(_("The Mupen64Plus core library was not found at <%s> or could not be loaded; please select it before you can continue"), corepath) );
            
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
            mplog_error("MupenAPI", "Sorry, a fatal error was caught :\n%s\n",  e.what());
            wxMessageBox( _("Sorry, initializing Mupen64Plus failed. Please verify the integrity of your installation.") );
            return false;
        }
    } // end while
    
    m_frame = new wxFrame(NULL, -1, "Mupen64Plus", wxDefaultPosition, wxSize(1024, 640));
    SetTopWindow(m_frame);
    
    wxInitAllImageHandlers();
    if (not makeToolbar(plugins, 0)) return false;
    
    m_status_bar = m_frame->CreateStatusBar();
    
    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    GamesPanel* games = new GamesPanel(m_frame, m_api, m_gamesPathParam);
    m_sizer->Add(games, 1, wxEXPAND);
    m_curr_panel = games;
    
    wxMenuBar* bar = new wxMenuBar();
    
    wxMenu* file = new wxMenu();
    file->Append(wxID_OPEN, _("&Open\tCtrl-O"));
    file->Append(wxID_ABOUT, _("&About"));
    file->Append(wxID_EXIT, _("&Quit") + "\tCtrl-Q");
    
    bar->Append(file, _("File"));
    m_frame->SetMenuBar(bar);
    
    m_frame->SetSizer(m_sizer);
    
    m_frame->Maximize();
    m_frame->Show();
    
    m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MupenFrontendApp::onClose), NULL, this);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    m_frame->Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
                     wxCommandEventHandler(MupenFrontendApp::onQuitMenu), NULL, this);
    
    m_frame->Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
                     wxCommandEventHandler(MupenFrontendApp::onAboutMenu), NULL, this);
    m_frame->Connect(wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED,
                     wxCommandEventHandler(MupenFrontendApp::onOpenMenu), NULL, this);
                     
    SetTopWindow( m_frame );
    Connect(wxID_ANY, wxEVT_ACTIVATE_APP, wxActivateEventHandler(MupenFrontendApp::onActivate), NULL, this);
    
    Connect(wxID_ANY, wxMUPEN_RELOAD_OPTIONS, wxCommandEventHandler(MupenFrontendApp::onReloadOptionsRequest), NULL, this);
    Connect(wxID_ANY, wxMUPEN_INIT_GL_CANVAS, wxCommandEventHandler(MupenFrontendApp::onInitGLCanvas), NULL, this);
    Connect(wxID_ANY, wxMUPEN_INITED_GL_CANVAS, wxCommandEventHandler(MupenFrontendApp::onInitedGLCanvas), NULL, this);
    Connect(wxID_ANY, wxMUPEN_CLEAN_GL_CANVAS, wxCommandEventHandler(MupenFrontendApp::onCleanGLCanvas), NULL, this);

    // check if filenames to open were given on the command-line
    for (int n=0; n<argc; n++)
    {
        wxString fileName = wxString(argv[n]);
        if (fileName.EndsWith(wxT("V64")) or fileName.EndsWith(wxT("v64")) or
            fileName.EndsWith(wxT("Z64")) or fileName.EndsWith(wxT("z64")))
        {
            openFile( fileName );
            break;
        }
    }

    Connect(wxID_ANY, wxMUPEN_READ_OPEN_FILE_QUEUE, wxCommandEventHandler(MupenFrontendApp::onReadOpenFileQueue), NULL, this);
    
#ifdef __WXMSW__
    // Drag files
    m_frame->Connect(wxID_ANY, wxEVT_DROP_FILES, wxDropFilesEventHandler(MupenFrontendApp::onDropFile),
                     NULL, this);
#endif

    wxCommandEvent evt(wxMUPEN_READ_OPEN_FILE_QUEUE);
    AddPendingEvent(evt);
    
    // enter the application's main loop
    return true;
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onReadOpenFileQueue(wxCommandEvent& evt)
{
    m_inited = true;
    
    if (m_pending_file_opens.size() > 0)
    {
        if (m_pending_file_opens.size() > 1)
        {
            wxMessageBox( _("You passed several ROMs to be opened; I can only open one at a time!") );
        }
        openFile(m_pending_file_opens[0]);
        m_pending_file_opens.clear();
    }
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onInitGLCanvas(wxCommandEvent& evt)
{
    ((GamesPanel*)m_curr_panel)->initGLCanvas();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onInitedGLCanvas(wxCommandEvent& evt)
{
    VidExt_InitedGLCanvas();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onCleanGLCanvas(wxCommandEvent& evt)
{
    ((GamesPanel*)m_curr_panel)->cleanGLCanvas();
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::shutdown()
{
    if (m_curr_panel != NULL)
    {
        m_curr_panel->commitNewValues(true);
        m_curr_panel->removeMyselfFrom(m_sizer);
        m_curr_panel = NULL;
    }

#ifdef ENABLE_DEBUGGER
    if (DebuggerFrame::Exists())
        DebuggerFrame::Delete();
#endif

    if (m_frame != NULL)
    {
        m_frame->Destroy();
        m_frame = NULL;
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
    try
    {
        if (m_api->getEmulationState() != M64EMU_STOPPED) evt.Veto();
        else                                              shutdown();
    }
    catch (std::exception& e)
    {
        mplog_warning("MupenFrontendApp", "Exception caught when getting emulation state : %s\n", e.what());
        shutdown();
    }
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onQuitMenu(wxCommandEvent& evt)
{
    try
    {
        if (m_api->getEmulationState() == M64EMU_STOPPED) shutdown();
    }
    catch (std::exception& e)
    {
        mplog_warning("MupenFrontendApp", "Exception caught when getting emulation state : %s\n", e.what());
        shutdown();
    }
}

// -----------------------------------------------------------------------------------------------------------

/** utility function */
wxString showFileDialog(wxString message, wxString defaultDir,
                        wxString filename,
                        wxString wildcard, bool save)
{
    wxFileDialog* dialog = new wxFileDialog( NULL, message, defaultDir, filename, wildcard, (save?wxFD_SAVE:wxFD_OPEN));
    int answer = dialog->ShowModal();
    wxString path = dialog->GetPath();
    dialog->Hide();
    dialog->Destroy();
    if (answer != wxID_OK) return wxT("");

    return path;
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onOpenMenu(wxCommandEvent& evt)
{
    wxString filePath = showFileDialog( _("Select file"), wxT(""), wxT(""), 
                                        _("N64 ROM") + "|*.v64;*.z64", false /*open*/);
    if (filePath.size() > 0)
    {
        openFile(filePath);
    }
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onAboutMenu(wxCommandEvent& evt)
{
    class wxHtmlWindowHelper : public wxHtmlWindow
    {
    public:
        wxHtmlWindowHelper(wxWindow* parent) : wxHtmlWindow(parent, wxID_ANY)
        {
            Connect(wxEVT_COMMAND_HTML_LINK_CLICKED,
                    wxHtmlLinkEventHandler(wxHtmlWindowHelper::onLinkClicked), NULL, this);
        }
        
        void onLinkClicked(wxHtmlLinkEvent& evt)
        {
            wxLaunchDefaultBrowser( evt.GetLinkInfo().GetHref() );
        }
    };
    
    wxBitmap icon_mupen(datadir + "mupenicon_large.png", wxBITMAP_TYPE_PNG);    
    wxDialog* about = new wxDialog(m_frame, wxID_ANY, _("About wxMupen64Plus"),
                                   wxDefaultPosition, wxSize(500, 400));
    
    wxStaticBitmap* icon = new wxStaticBitmap(about, wxID_ANY, icon_mupen);
    
    wxStaticText* label = new wxStaticText(about, wxID_ANY,
            wxString::Format("wxMupen64Plus %i.%i", VERSION_MAJOR, VERSION_MINOR) );
    label->SetFont( label->GetFont().MakeBold().Scaled(2.0f) );
    
    wxString about_text = wxString("<html<body><p align=\"center\">") +
                          _("A wxWidgets-based frontend for the <b>Mupen64Plus</b> v2 emulator") +
                          "<br><br>" + _("by Marianne Gagnon") +
                          "<br><br><a href=\"https://bitbucket.org/auria/wxmupen64plus/wiki\">" +
                          "https://bitbucket.org/auria/wxmupen64plus/wiki</a></p></body></html>";
    
    wxHtmlWindowHelper* text_area = new wxHtmlWindowHelper(about);
    text_area->SetPage(about_text);
    
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    s->AddSpacer(20);
    s->Add(icon, 0, wxALIGN_CENTER | wxALL, 5);
    s->Add(label, 0, wxALIGN_CENTER | wxALL, 5);
    s->AddSpacer(30);
    s->Add(text_area, 1, wxEXPAND);
    about->SetSizer(s);
    
    about->CenterOnParent();
    about->Layout();
    about->ShowModal();
    
    //text_area->Disconnect(wxEVT_COMMAND_HTML_LINK_CLICKED,
    //                      wxHtmlLinkEventHandler(MupenFrontendApp::onLinkClicked), this, NULL);
    //s->Detach(text_area);
    //text_area->Destroy();
    
    about->Destroy();
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

void MupenFrontendApp::openFile(const wxString &filename)
{
    printf("Openfile, m_inited=%i\n", m_inited);
    if (not m_inited)
    {
        m_pending_file_opens.push_back(filename);
        return;
    }
    
    if (dynamic_cast<GamesPanel*>(m_curr_panel) != NULL)
    {
        GamesPanel* gp = dynamic_cast<GamesPanel*>(m_curr_panel);
        gp->loadRom(wxFileName::FileName(filename).GetName(), filename);
    }
    else
    {
        // TODO: file associations don't work when you're not on the games panel
    }
}

// -----------------------------------------------------------------------------------------------------------

#ifdef __WXMAC__

void MupenFrontendApp::MacOpenFile(const wxString &filename)
{
    openFile(filename);
}

#endif

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onDropFile(wxDropFilesEvent& evt)
{
    int i;
    wxString fileName;
    bool firstFound = false;
    for (i=0 ; i<evt.GetNumberOfFiles() ;i++)
    {
        fileName = evt.m_files[i];
        
        if (firstFound)
        {
            wxLogWarning(_("Can only open one file at a time, ignoring : '%s'"),
                         (const char*)fileName.mb_str());        
        }
        else if (fileName.EndsWith("z64") or fileName.EndsWith("Z64") or
                 fileName.EndsWith("v64") or fileName.EndsWith("V64"))
        {
            firstFound = true;
            openFile(fileName);
        }
        else
        {
            wxLogWarning(_("Cannot open file of unknown type : '%s'"), (const char*)fileName.mb_str());
        }
    }
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
    if (m_api->getEmulationState() == M64EMU_RUNNING or m_api->getEmulationState() == M64EMU_PAUSED)
    {
        wxBell();
        return;
    }
    
#ifndef __WXGTK__
    // FIXME: for some reason, Freeze and Thaw cause some random hanging on wxGTK. For now I'll just disable
    //        them in this case, anyway freeze and thaw don't appear to be needed on this platform
    m_frame->Freeze();
#endif
    const int id = evt.GetId();

    // Find which section was clicked
    int sectionId = -1;
    const int count = m_toolbar_items.size();
    for (int n=0; n<count; n++)
    {
        if (m_toolbar_items[n].m_tool->GetId() == id)
        {
            if (not evt.IsChecked())
            {
                m_toolbar->ToggleTool(id, true);
            }
            
            if (m_current_panel == n)
            {
                // This section is already selected
#ifndef __WXGTK__
                m_frame->Thaw();
#endif
                return;
            }
    
            sectionId = n;
            break;
        }
    }

    if (sectionId == -1)
    {
#ifndef __WXGTK__
        m_frame->Thaw();
#endif
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

#ifndef __WXGTK__
    m_frame->Thaw();
#endif
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
    
    wxIcon   icon_mupen  (datadir + "mupenicon.png", wxBITMAP_TYPE_PNG);
    wxBitmap icon_input  (datadir + "input.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_cpu    (datadir + "emulation.png", wxBITMAP_TYPE_PNG);
    wxBitmap icon_audio  (datadir + "audio.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_plugins(datadir + "plugins.png",   wxBITMAP_TYPE_PNG);
    wxBitmap icon_video  (datadir + "video.png",     wxBITMAP_TYPE_PNG);
    wxBitmap icon_other  (datadir + "other.png",     wxBITMAP_TYPE_PNG);
    
#ifndef __WXMAC__
    // while we have the icpon let's use it... (except on OSX where an application-wide icon is
    // already configured into the app bundle, so no need for this call there)
    m_frame->SetIcon( icon_mupen );
#endif

    if (not icon_mupen.IsOk()) icon_mupen = wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_input.IsOk()) icon_input = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_cpu.IsOk()) icon_cpu = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_audio.IsOk()) icon_audio = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_plugins.IsOk()) icon_plugins = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_video.IsOk()) icon_video = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
    if (not icon_other.IsOk()) icon_other = wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(32,32));
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
                ptr_param->setEnabled( false );
                m_gamesPathParam = ptr_param;
            }
            
            ConfigParam* pluginsDir = section->getParamWithName("PluginDir");
            if (pluginsDir != NULL)
            {
                ConfigParam* plugin1 = section->getParamWithName("VideoPlugin");
                if (plugin1 != NULL)
                {
                    plugin1->m_special_type = PLUGIN_FILE;
                    plugin1->m_plugin_type = M64PLUGIN_GFX;
                    plugin1->m_is_ok = (plugins & 0x1) != 0;
                    plugin1->m_dir = new ConfigParam(*pluginsDir);
                }
                
                ConfigParam* plugin2 = section->getParamWithName("AudioPlugin");
                if (plugin2 != NULL)
                {
                    plugin2->m_special_type = PLUGIN_FILE;
                    plugin2->m_is_ok = (plugins & 0x2) != 0;
                    plugin2->m_dir = new ConfigParam(*pluginsDir);
                    plugin2->m_plugin_type = M64PLUGIN_AUDIO;
                }
                
                ConfigParam* plugin3 = section->getParamWithName("InputPlugin");
                if (plugin3 != NULL)
                {
                    plugin3->m_special_type = PLUGIN_FILE;
                    plugin3->m_is_ok = (plugins & 0x4) != 0;
                    plugin3->m_dir = new ConfigParam(*pluginsDir);
                    plugin3->m_plugin_type = M64PLUGIN_INPUT;
                }
                
                ConfigParam* plugin4 = section->getParamWithName("RspPlugin");
                if (plugin4 != NULL)
                {
                    plugin4->m_special_type = PLUGIN_FILE;
                    plugin4->m_is_ok = (plugins & 0x8) != 0;
                    plugin4->m_dir = new ConfigParam(*pluginsDir);
                    plugin4->m_plugin_type = M64PLUGIN_RSP;
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
            mplog_warning("Config","Ignoring config section %s\n", section->m_section_name.c_str());
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

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::enableToolbar(bool enable)
{
    for (unsigned int n=0; n<m_toolbar_items.size(); n++)
    {
        m_toolbar->EnableTool( m_toolbar_items[n].m_tool->GetId(), enable );        
    }
    
}

// -----------------------------------------------------------------------------------------------------------

void MupenFrontendApp::onReloadOptionsRequest(wxCommandEvent& evt)
{
    // On wxOSX, changing the toolbar has the ugly disadvantage of resizing the frame...
    m_frame->Freeze();
#ifdef __WXMAC__
    wxSize size = m_frame->GetSize();
#endif

    manualRemoveCurrentPanel();
    makeToolbar(evt.GetInt(), m_current_panel);
    manualReshowCurrentPanel();
    
#ifdef __WXMAC__
    m_frame->SetSize(size);
#endif

    m_frame->Thaw();
    
#ifdef __WXMSW__
    m_frame->Layout();
#endif
}

// -----------------------------------------------------------------------------------------------------------


#define RESET    "\033[0m"
#define BOLD     "\033[1m"
#define GREY     "\033[1;30m"
#define RED      "\033[31m"
#define YELLOWBG "\033[33m"

extern "C"
{
    
    void mplog_info(const char* who, const char* message, ...)
    {
        va_list argp;
        va_start(argp, message);
        printf("[" BOLD GREY "%s" RESET "] ", who);
        vprintf(message, argp);
        va_end(argp);
    }

    void mplog_warning(const char* who, const char* message, ...)
    {
        va_list argp;
        va_start(argp, message);
        printf("[" BOLD YELLOWBG "%s" RESET "] " YELLOWBG, who);
        vprintf(message, argp);
        printf(RESET);
        va_end(argp);
    }

    void mplog_error(const char* who, const char* message, ...)
    {
        printf("[" BOLD RED "%s" RESET "] " RED, who);
        
        va_list argp;
        va_start(argp, message);
        vprintf(message, argp);
        printf(RESET);
        va_end(argp);
    }

}
