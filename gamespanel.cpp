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

#include "gamespanel.h"
#include "mupen64plusplus/MupenAPIpp.h"

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

#include <stdexcept>
#include <map>

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

 
int wxCALLBACK GamesPanel::wxListCompareFunction(long item1, long item2, wxIntPtr sortData)
{
    GamesPanel* self = (GamesPanel*)sortData;
    RomInfo& rom1 = self->m_roms[item1];
    RomInfo& rom2 = self->m_roms[item2];
    
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
    m_gamesPathParam = gamesPathParam;
    api->setListener(this);
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
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
    
    m_dir_picker = new wxDirPickerCtrl(this, wxID_ANY, path);
    sizer->Add(m_dir_picker, 0, wxALL | wxEXPAND, 5);

    m_dir_picker->Connect(m_dir_picker->GetId(), wxEVT_COMMAND_DIRPICKER_CHANGED,
                          wxFileDirPickerEventHandler(GamesPanel::onPathChange), NULL, this);
    
    m_item_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    sizer->Add(m_item_list, 1, wxALL | wxEXPAND, 5);
    
    populateList();
    
    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    
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
    
    wxBitmap icon_stop(datadir + "stop.png", wxBITMAP_TYPE_PNG);  
    m_stop_button = new wxBitmapButton(this, wxID_ANY, icon_stop, wxDefaultPosition, wxDefaultSize,
                                       wxBORDER_NONE);
    buttons->Add(m_stop_button, 0, wxALL, 5);
    
    buttons->AddStretchSpacer();
    
    m_status = new wxStaticText(this, wxID_ANY, _("Emulation is stopped"));
    buttons->Add(m_status, 0, wxALIGN_CENTER_VERTICAL  | wxALL, 5);
    
    wxBitmap icon_cart(datadir + "mupen64cart.png", wxBITMAP_TYPE_PNG);  
    wxStaticBitmap* icon = new wxStaticBitmap(this, wxID_ANY, icon_cart);
    buttons->Add(icon, 0, wxALL, 5);
    
    
    sizer->Add(buttons, 0, wxEXPAND | wxALL, 5);
    

    
    m_play_button->Connect(m_play_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(GamesPanel::onPlay), NULL, this);
    m_pause_button->Connect(m_pause_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(GamesPanel::onPause), NULL, this);
    m_stop_button->Connect(m_stop_button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(GamesPanel::onStop), NULL, this);
    m_pause_button->Disable();
    m_stop_button->Disable();
    SetSizer(sizer);
    
    m_item_list->Connect(m_item_list->GetId(), wxEVT_COMMAND_LIST_COL_CLICK,
                         wxListEventHandler(GamesPanel::onColClick), NULL, this);
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
    m_item_list->ClearAll();
        
    if (path.IsEmpty()) return;
    
    wxArrayString columns;              std::vector<int> sizes;
    columns.Add( _("File Name") );      sizes.push_back( 300 );
    columns.Add( _("Internal Name") );  sizes.push_back( 225 );
    //columns.Add( _("Good Name") );      sizes.push_back( 225 );
    columns.Add( _("Country") );        sizes.push_back( 75 );
    //columns.Add( _("Size") );        sizes.push_back( 75 );
    
    const int count = columns.Count();
    for (int n=0; n<count; n++)
    {
        wxListItem col;
        col.SetId(n);
        col.SetText( columns[n] );
        col.SetWidth( sizes[n] );
        m_item_list->InsertColumn(n, col);
    }
    
    m_roms = getRomsInDir(path);
    
    const int item_amount = m_roms.size();
    for (int n=0; n<item_amount; n++)
    {
        RomInfo& curritem = m_roms[n];
                
        wxListItem item;
        item.SetId(n);
        item.SetText( curritem.m_file_name );
        
        Mupen64PlusPlus::RomInfo info;
        
        long id = m_item_list->InsertItem( item );
        
        if (id != -1)
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
                    fprintf(stderr, "Failed to load rom %s : %s",
                                    (const char*)curritem.m_full_path.utf8_str(),
                                    ex.what());
                }
            }
            
            curritem.m_country = info.country;
            curritem.m_internal_name = info.name;
        
            // set value in first column
            m_item_list->SetItem(id, COLUMN_FILE, curritem.m_file_name);
            
            // set value in second column
            m_item_list->SetItem(id, COLUMN_NAME, info.name);
            
            // set value in third column
            //m_item_list->SetItem(id, 2, info.goodname);
            
            // set value in fourth column
            m_item_list->SetItem(id, COLUMN_COUNTRY, info.country);
            
            m_item_list->SetItemData(id, id);
        }
    } // end for
    
    m_item_list->SortItems(GamesPanel::wxListCompareFunction, (wxIntPtr)this /* user data */);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onColClick(wxListEvent& evt)
{
    // FIXME: wxListCtrl is awful, the native OSX list ordering when clicking on columns is not controllable
    // (not portable I think)
    if (m_curr_col != evt.GetColumn())
    {
        m_curr_col = evt.GetColumn();
        m_item_list->SortItems(GamesPanel::wxListCompareFunction, (wxIntPtr)this /* user data */);
    }  
}

// -----------------------------------------------------------------------------------------------------------

std::vector<GamesPanel::RomInfo> GamesPanel::getRomsInDir(wxString dirpath)
{
    std::vector<RomInfo> out;
    
    wxArrayString children;
    // wxDIR_FILES is used to avoid very long processing (e.g. searching the entire disk if path is set to /...)
    // TODO: allow limited recursion into directories?
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
    
    long item = m_item_list->GetNextItem(-1,
                                        wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
                                        
    if (item == -1)
    {
        wxMessageBox( _("No game is selected, cannot start emulation") );
        return;
    }
    
    wxProgressDialog dialog( _("Loading..."), _("Your game is loading") );
    dialog.Show();
    
    wxString file = path + wxFileName::GetPathSeparator() + m_item_list->GetItemText(item);
    printf("\n==== Running file '%s' ====\n\n", (const char*)file.utf8_str());

    try
    {
        m_api->loadRom(file, true, &dialog);
        m_currently_loaded_rom = m_item_list->GetItemText(item);
        dialog.Hide();
        m_api->runEmulation();
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
        m_api->closeRom();
        m_currently_loaded_rom = "";
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox( wxString(_("An error occurred while trying to stop emulation :")) + ex.what() );
    }
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onStateChanged(m64p_emu_state newState)
{
    switch (newState)
    {
        case M64EMU_STOPPED:
            if (m_api->isARomOpen()) m_api->closeRom();
            m_play_button->Enable();
            m_stop_button->Disable();
            m_pause_button->Disable();
            m_status->SetLabel(_("Emulation is stopped"));
            Layout();
            break;
        
        case M64EMU_RUNNING:
            m_play_button->Disable();
            m_stop_button->Enable();
            m_pause_button->Enable();
            m_status->SetLabel(wxString::Format(_("'%s' is running"), m_currently_loaded_rom.mb_str()));
            Layout();
            break;
        
        case M64EMU_PAUSED:
            m_play_button->Enable();
            m_stop_button->Enable();
            m_pause_button->Disable();
            m_status->SetLabel(wxString::Format(_("'%s' is paused"), m_currently_loaded_rom.mb_str()));
            Layout();
            break;
    }
}

// -----------------------------------------------------------------------------------------------------------
