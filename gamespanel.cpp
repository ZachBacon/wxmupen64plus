/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   wxMupen64Plus frontend                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marianne Gagnon                                    *
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

#ifdef __WXMSW__
#include <wx/wx.h>
#endif

#include "gamespanel.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "main.h"
#include "wxvidext.h"

#include "sdlkeypicker.h"
#include <wx/sizer.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/bmpbuttn.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/msgqueue.h>
#include <wx/progdlg.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/frame.h>
#include <wx/glcanvas.h>
#include <wx/display.h>

#include <stdexcept>
#include <map>

DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_STATE_CHANGE);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_SAVE_SLOT_CHANGE);
DEFINE_LOCAL_EVENT_TYPE(wxMUPEN_CLEANUP_GL_CANVAS);

enum
{
	COLUMN_FILE = 0,
	COLUMN_NAME,
	COLUMN_COUNTRY
};

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- GAMES CACHE -----------------------------------------------
// To avoid disk access, cache ROM info after reading a rom header
std::map<wxString, Mupen64PlusPlus::RomInfo> g_cache;

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- GAMES PANEL -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// The main part where the game list is shown

// FIXME: wxListCtrl is awful, the native OSX list ordering when clicking on columns is not controllable
// (nor portable I think)
int wxCALLBACK GamesPanel::wxListCompareFunction(long item1, long item2, wxIntPtr sortData)
{
    GamesPanel* self = (GamesPanel*)sortData;
    
#if defined(__WXMAC__) && !defined(__WXOSX_COCOA__)
    RomInfo& rom1 = self->m_roms[item1];
    RomInfo& rom2 = self->m_roms[item2];
#else
    RomInfo& rom1 = self->m_roms[item2];
    RomInfo& rom2 = self->m_roms[item1];
#endif

    //printf("Comparing <%s> and <%s>\n", (const char*)rom1.m_file_name.mb_str(),
    //    (const char*)rom2.m_file_name.mb_str());
    
    if (self->m_curr_col == COLUMN_FILE) 
    {
        return rom2.m_file_name.CmpNoCase( rom1.m_file_name );
    }
    else if (self->m_curr_col == COLUMN_NAME)
    {
        return rom2.m_internal_name.CmpNoCase( rom1.m_internal_name );
    }
    else if (self->m_curr_col == COLUMN_COUNTRY)
    {
        return rom2.m_country.CmpNoCase( rom1.m_country );
    }
    else
    {
        fprintf(stderr, "Unknown column %i?\n", self->m_curr_col);
        return rom2.m_file_name.CmpNoCase( rom1.m_file_name );
    }
}

// -----------------------------------------------------------------------------------------------------------

GamesPanel::GamesPanel(wxWindow* parent, Mupen64PlusPlus* api, ConfigParam* gamesPathParam) :
        wxPanel(parent, wxID_ANY)
{
    m_curr_col = 0;
    m_api = api;
    m_canvas = NULL;
    m_gamesPathParam = gamesPathParam;
    api->setListener(this);
    
    m_width_param = NULL;
    m_height_param = NULL;
    
    wxBoxSizer* oversizer = new wxBoxSizer(wxHORIZONTAL);
    
    // ---- Buttons area
    wxBoxSizer* buttons = new wxBoxSizer(wxVERTICAL);
    
    buttons->AddStretchSpacer();
    
#ifdef DATADIR
    wxString datadir = wxString(DATADIR) + wxFileName::GetPathSeparator();
#else
    wxString datadir = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
#endif

    wxBitmap icon_play(datadir + "play.png", wxBITMAP_TYPE_PNG);  
    m_play_button = new wxBitmapButton(this, wxID_ANY, icon_play, wxDefaultPosition, wxDefaultSize,
                                       wxBORDER_NONE);
    buttons->Add(m_play_button, 0, wxALL, 5);
    
    wxBitmap icon_pause(datadir + "pause.png", wxBITMAP_TYPE_PNG);  
    m_pause_button = new wxBitmapButton(this, wxID_ANY, icon_pause, wxDefaultPosition, wxDefaultSize,
                                       wxBORDER_NONE);
    buttons->Add(m_pause_button, 0, wxALL, 5);
    
	m_pause_button->SetCanFocus(false);
	m_pause_button->Connect(m_pause_button->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(GamesPanel::wanderingFocus), NULL, this);
	
    wxBitmap icon_stop(datadir + "stop.png", wxBITMAP_TYPE_PNG);  
    m_stop_button = new wxBitmapButton(this, wxID_ANY, icon_stop, wxDefaultPosition, wxDefaultSize,
                                       wxBORDER_NONE);
    buttons->Add(m_stop_button, 0, wxALL, 5);
    
	m_stop_button->SetCanFocus(false);
	m_stop_button->Connect(m_stop_button->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(GamesPanel::wanderingFocus), NULL, this);
	
    buttons->AddStretchSpacer();
    
#ifdef __WXMSW__
    // On Windows, the default disabled look is indisinguishible from the enabled look, not sure
    // why... maybe it converts the image to greyscale but this is already a greyscale image
    m_play_button->SetBitmapDisabled( wxBitmap( icon_play.ConvertToImage().ConvertToDisabled(100) ) );
    m_pause_button->SetBitmapDisabled( wxBitmap( icon_pause.ConvertToImage().ConvertToDisabled(100) ) );
    m_stop_button->SetBitmapDisabled( wxBitmap( icon_stop.ConvertToImage().ConvertToDisabled(100) ) );    
#endif

    // ---- List area
    m_list_sizer = new wxBoxSizer(wxVERTICAL);
    
    wxString path;
    //if (m_gamesPathParam != NULL)
    {
        try
        {
            path = m_gamesPathParam->getStringValue();
        }
        catch (std::runtime_error& ex)
        {
            wxLogWarning("Failed to read ROMs path from config file : %s", ex.what());
        }
    }
    
    m_center_panel = new wxPanel(this);
    
    m_dir_picker = new wxDirPickerCtrl(m_center_panel, wxID_ANY, path, _("Directory Picker"), wxDefaultPosition, wxDefaultSize,
                                       wxDIRP_DEFAULT_STYLE | wxDIRP_USE_TEXTCTRL);
    m_list_sizer->Add(m_dir_picker, 0, wxALL | wxEXPAND, 5);

    m_dir_picker->Connect(m_dir_picker->GetId(), wxEVT_COMMAND_DIRPICKER_CHANGED,
                          wxFileDirPickerEventHandler(GamesPanel::onPathChange), NULL, this);
    
    m_item_list = new wxDataViewListCtrl(m_center_panel, wxID_ANY);
    m_list_sizer->Add(m_item_list, 1, wxALL | wxEXPAND, 5);
    
    
    wxArrayString columns;              std::vector<int> sizes;
    columns.Add( _("File Name") );      sizes.push_back( 380 );
    columns.Add( _("Internal Name") );  sizes.push_back( 250 );
    columns.Add( _("Country") );        sizes.push_back( 100 );
    
    const int count = columns.Count();
    for (int n=0; n<count; n++)
    {
        m_item_list->AppendColumn(new wxDataViewColumn(columns[n], new wxDataViewTextRenderer(), n,
                                  sizes[n], wxALIGN_LEFT,
                                  wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE ));
    }
    
    
    populateList();
    
    m_center_panel->SetSizer(m_list_sizer);
    
    oversizer->Add(m_center_panel, 1, wxEXPAND);
    oversizer->Add(buttons, 0, wxEXPAND | wxRIGHT, 5);
    
    SetSizer(oversizer);
    
    // ---- Events ----
    m_play_button->Connect(m_play_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(GamesPanel::onPlay), NULL, this);
    m_pause_button->Connect(m_pause_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(GamesPanel::onPause), NULL, this);
    m_stop_button->Connect(m_stop_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(GamesPanel::onStop), NULL, this);
    m_pause_button->Disable();
    m_stop_button->Disable();
    
    /*
    m_item_list->Connect(m_item_list->GetId(), wxEVT_COMMAND_LIST_COL_CLICK,
                         wxListEventHandler(GamesPanel::onColClick), NULL, this);
    */
    
    Connect(wxID_ANY, wxMUPEN_STATE_CHANGE,
            wxCommandEventHandler(GamesPanel::onMupenStateChangeEvt), NULL, this);
    Connect(wxID_ANY, wxMUPEN_SAVE_SLOT_CHANGE,
            wxCommandEventHandler(GamesPanel::onMupenSaveSlotChangeEvt), NULL, this);
    Connect(wxID_ANY, wxMUPEN_CLEANUP_GL_CANVAS,
            wxCommandEventHandler(GamesPanel::onCleanGLCanvas), NULL, this);
}

// -----------------------------------------------------------------------------------------------------------

GamesPanel::~GamesPanel()
{
    m_api->setListener(NULL);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::populateList()
{
    wxString path = m_dir_picker->GetPath();
    m_item_list->DeleteAllItems();
    
    if (path.IsEmpty()) return;
    
    m_roms = getRomsInDir(path);
    
    const int item_amount = m_roms.size();
    for (int n=0; n<item_amount; n++)
    {
        RomInfo& curritem = m_roms[n];
        
        /*
        wxListItem item;
        item.SetId(n);
        item.SetText( curritem.m_file_name );
        */
        Mupen64PlusPlus::RomInfo info;
        
        //long id = m_item_list->InsertItem( item );
        
        //if (id != -1)
        {
            std::map<wxString, Mupen64PlusPlus::RomInfo>::iterator elem = g_cache.find(curritem.m_full_path);
            if (elem != g_cache.end())
            {
                // Element found in cache, great
                info = g_cache[curritem.m_full_path];
            }
            else
            {
                try
                {
                    info = m_api->getRomInfo(curritem.m_full_path.utf8_str());
                }
                catch (std::runtime_error& ex)
                {
                    mplog_error("GamesPanel", "Failed to load rom %s : %s",
                                (const char*)curritem.m_full_path.utf8_str(),
                                ex.what());
                }
            }
            
            curritem.m_country = info.country;
            curritem.m_internal_name = info.name;
        
        /*
            // set value in first column
            m_item_list->SetItem(id, COLUMN_FILE, curritem.m_file_name);
            
            // set value in second column
            m_item_list->SetItem(id, COLUMN_NAME, info.name);
            
            // set value in third column
            //m_item_list->SetItem(id, 2, info.goodname);
            
            // set value in fourth column
            m_item_list->SetItem(id, COLUMN_COUNTRY, info.country);
            
            m_item_list->SetItemData(id, id);
            */
            
            wxVector< wxVariant > values;
            values.push_back( wxVariant(curritem.m_file_name) );
            values.push_back( wxVariant(curritem.m_internal_name) );
            values.push_back( wxVariant(curritem.m_country) );
            m_item_list->AppendItem(values);
        }
    } // end for
    
    //m_item_list->SortItems(GamesPanel::wxListCompareFunction, (wxIntPtr)this /* user data */);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::initGLCanvas()
{
    Freeze();
    m_canvas = VidExt_InitGLCanvas(m_center_panel);
    if (m_canvas != NULL)
    {
        m_item_list->Hide();
        m_dir_picker->Hide();
        m_list_sizer->Add(m_canvas, 1, wxEXPAND | wxALL, 5);
        m_list_sizer->SetSizeHints(m_canvas);
        m_center_panel->Layout();
        Layout();
        Thaw();
        m_canvas->SetFocus();
        //m_canvas->Connect(wxID_ANY, wxEVT_KILL_FOCUS, wxFocusEventHandler(GamesPanel::wanderingFocus), NULL, this);

        ((wxFrame*)GetParent())->Layout();
        ((wxFrame*)GetParent())->Refresh();    

        // FIXME: ugly hack to force a full refresh
        ((wxFrame*)GetParent())->SetSize( ((wxFrame*)GetParent())->GetSize() );
    }
    
    wxCommandEvent evt(wxMUPEN_INITED_GL_CANVAS, -1);
    wxGetApp().AddPendingEvent(evt);
	
    if (m_canvas != NULL)
    {
        m_canvas->SetFocus();
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::wanderingFocus(wxFocusEvent& evt)
{
	if (m_canvas != NULL) m_canvas->SetFocus();
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::cleanGLCanvas()
{
    VidExt_AsyncCleanup();
    if (m_canvas)
    {
        Freeze();
        m_item_list->Show();
        m_dir_picker->Show();
        m_list_sizer->Detach(m_canvas);
        m_canvas->Destroy();
        m_canvas = NULL;
        Layout();
        Thaw();
    }
    Refresh();
}

// -----------------------------------------------------------------------------------------------------------
#if 0
void GamesPanel::onColClick(wxListEvent& evt)
{
    if (m_curr_col != evt.GetColumn())
    {
        m_curr_col = evt.GetColumn();
        m_item_list->SortItems(GamesPanel::wxListCompareFunction, (wxIntPtr)this /* user data */);
    }  
}
#endif

// -----------------------------------------------------------------------------------------------------------

std::vector<GamesPanel::RomInfo> GamesPanel::getRomsInDir(wxString dirpath)
{
    std::vector<RomInfo> out;
    
    wxArrayString children;
    // wxDIR_FILES is used to avoid very long processing (e.g. searching the entire disk if path is set to /...)
    const int count = wxDir::GetAllFiles(dirpath, &children, wxEmptyString, wxDIR_FILES);
    
    assert(count == (int)children.Count());
    
    for (int n=0; n<count; n++)
    {
        wxString filename = wxFileName::FileName(children[n]).GetFullName();
        
        // roms typically end with "*.n64|*.z64|*.v64"
        if (filename.EndsWith("64") && !filename.StartsWith("."))
        {
            out.push_back( RomInfo(children[n],
                                   filename, "", "") );
        }
    }
    
    return out;
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPathChange(wxFileDirPickerEvent& event)
{
    //killThread();
    
    try
    {
        m_gamesPathParam->setStringValue(m_dir_picker->GetPath().ToStdString());
    }
    catch (std::runtime_error& ex)
    {
        wxLogWarning("Failed to save ROM path to config file : %s", ex.what());
    }

    populateList();
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPlay(wxCommandEvent& evt)
{
    //killThread();
    
    if (m_api->getEmulationState() == M64EMU_PAUSED)
    {
        m_api->resumeEmulation();
        return;
    }
    
    wxString path = m_dir_picker->GetPath();        
    if (path.IsEmpty())
    {
        wxBell();
        return;
    }
    
    ptr_vector<ConfigSection>& config = wxGetApp().getConfig();
    for (int n=0; n<config.size(); n++)
    {
        if (config[n]->m_section_name == "General")
        {
            
            m_width_param = config[n]->getParamWithName("ScreenWidth");
            m_height_param = config[n]->getParamWithName("ScreenHeight");
            m_fullscreen_param = config[n]->getParamWithName("Fullscreen");            
            break;
        }
    }
    
    if (m_width_param == NULL or m_height_param == NULL)
    {
        mplog_warning("GamesPanel", "Cannot find size parameters\n");
    }
    else
    {
        m_previous_width = m_width_param->getIntValue();
        m_previous_height = m_height_param->getIntValue();
        
        if (m_fullscreen_param->getBoolValue())
        {
            int displayID = wxDisplay::GetFromWindow( wxGetApp().GetTopWindow() );
            if (displayID == wxNOT_FOUND)
            {
                wxMessageBox( _("An internal error occurred : can't determine on which display the frame is") );
                return;
            }
            wxDisplay display(displayID);
            
            // maximize (but keep aspect ratio)
            m_width_param->setIntValue(display.GetGeometry().GetHeight()*1.33f);
            m_height_param->setIntValue(display.GetGeometry().GetHeight());
        }
        else
        {
            if (m_api->useVideoExtension())
            {
                // maximize (but keep aspect ratio)
                m_width_param->setIntValue(m_center_panel->GetSize().GetHeight()*1.33f);
                m_height_param->setIntValue(m_center_panel->GetSize().GetHeight());
            }
        }
    }
    
    //long item = m_item_list->GetNextItem(-1,
    //                                    wxLIST_NEXT_ALL,
    //                                    wxLIST_STATE_SELECTED);
    
    int item = m_item_list->GetSelectedRow();
    
    if (item == wxNOT_FOUND)
    {
        wxMessageBox( _("No game is selected, cannot start emulation") );
        return;
    }
    
    wxString romfile = m_item_list->GetTextValue(item, 0);
    wxString file = path + wxFileName::GetPathSeparator() + romfile; //m_item_list->GetItemText(item);
    loadRom(romfile, file);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::loadRom(wxString name, wxString file)
{    
    if (m_api->getEmulationState() != M64EMU_STOPPED)
    {
        wxMessageBox( _("A game is already running") );
        return;
    }
    
    wxProgressDialog dialog( _("Loading..."), _("Your game is loading") );
    dialog.Show();
    
    printf("\n==== Running file '%s' ====\n\n", (const char*)file.utf8_str());

    try
    {        
        m_api->loadRom(file, true, &dialog);
        
        m_currently_loaded_rom = name;
        dialog.Hide();
        #ifdef __WXMAC__
        const bool asynchronous = false;
        #else
        const bool asynchronous = true;
        #endif
        
        setOsdLogging(true);
        
        m_api->runEmulation(asynchronous);
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox(ex.what());
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPause(wxCommandEvent& evt)
{
    try
    {
        m_api->pauseEmulation();
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox( wxString(_("An error occurred while trying to pause emulation :")) + ex.what() );
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onStop(wxCommandEvent& evt)
{
    try
    {
        m_api->stopEmulation();
        
        wxCommandEvent evt(wxMUPEN_CLEANUP_GL_CANVAS, -1);
        wxGetApp().AddPendingEvent(evt);
        //cleanGLCanvas();
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox( wxString(_("An error occurred while trying to stop emulation :")) + ex.what() );
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onMupenStateChangeEvt(wxCommandEvent& evt)
{
    m64p_emu_state newState = (m64p_emu_state)evt.GetInt();
    
    switch (newState)
    {
        case M64EMU_STOPPED:
        
            setOsdLogging(false);
        
            if (m_width_param != NULL and m_height_param != NULL and m_api->useVideoExtension())
            {
                m_width_param->setIntValue(m_previous_width);
                m_height_param->setIntValue(m_previous_height);
            }
    
            if (m_api->isARomOpen()) m_api->closeRom();
            m_currently_loaded_rom = "";
            m_play_button->Enable();
            m_stop_button->Disable();
            m_pause_button->Disable();
            ((wxFrame*)GetParent())->GetStatusBar()->SetStatusText(_("Emulation is stopped"));
            wxGetApp().enableToolbar(true);
            //m_status->SetLabel(_("Emulation is stopped"));
            //Layout();
            
            cleanGLCanvas();
            break;
        
        case M64EMU_RUNNING:
            
            setOsdLogging(true);
            
            m_play_button->Disable();
            m_stop_button->Enable();
            m_pause_button->Enable();
            ((wxFrame*)GetParent())->GetStatusBar()->SetStatusText(wxString::Format(_("'%s' is running"), m_currently_loaded_rom.mb_str()));
            wxGetApp().enableToolbar(false);
            //m_status->SetLabel(wxString::Format(_("'%s' is running"), m_currently_loaded_rom.mb_str()));
            Layout();
            break;
        
        case M64EMU_PAUSED:
            m_play_button->Enable();
            m_stop_button->Enable();
            m_pause_button->Disable();
            ((wxFrame*)GetParent())->GetStatusBar()->SetStatusText(wxString::Format(_("'%s' is paused"), m_currently_loaded_rom.mb_str()));
            wxGetApp().enableToolbar(false);
            //m_status->SetLabel(wxString::Format(_("'%s' is paused"), m_currently_loaded_rom.mb_str()));
            Layout();
            break;
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onMupenSaveSlotChangeEvt(wxCommandEvent& evt)
{
    //const int slot = evt.GetInt();
    // For now we don't do anything
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onStateChanged(m64p_emu_state newState)
{
    // This may be invoked from a thread, so send an event asynchronously to the main (GUI) thread
    wxCommandEvent evt(wxMUPEN_STATE_CHANGE, wxID_ANY);
    evt.SetInt(newState);
    GetEventHandler()->AddPendingEvent(evt);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onSaveSlotChanged(int saveSlot)
{
    // This may be invoked from a thread, so send an event asynchronously to the main (GUI) thread
    wxCommandEvent evt(wxMUPEN_SAVE_SLOT_CHANGE, wxID_ANY);
    evt.SetInt(saveSlot);
    GetEventHandler()->AddPendingEvent(evt);
}
