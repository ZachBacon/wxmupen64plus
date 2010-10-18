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

#include <stdexcept>

// -----------------------------------------------------------------------------------------------------------

GamesPanel::GamesPanel(wxWindow* parent, Mupen64PlusPlus* api) : wxPanel(parent, wxID_ANY)
{
    m_api = api;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // TODO: retrieve ROM path from config
    m_dir_picker = new wxDirPickerCtrl(this, wxID_ANY, "");
    sizer->Add(m_dir_picker, 0, wxALL | wxEXPAND, 5);

    m_dir_picker->Connect(m_dir_picker->GetId(), wxEVT_COMMAND_DIRPICKER_CHANGED,
                          wxFileDirPickerEventHandler(GamesPanel::onPathChange), NULL, this);
    
    m_item_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES);
    sizer->Add(m_item_list, 1, wxALL | wxEXPAND, 5);
    
    populateList();
    
    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    
    // FIXME: don't duplicate this line from main.cpp
    wxString resources = wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator();
    
    wxBitmap icon_play(resources + "play.png", wxBITMAP_TYPE_PNG);  
    wxBitmapButton* playBtn = new wxBitmapButton(this, wxID_ANY, icon_play, wxDefaultPosition, wxDefaultSize,
                                                 wxBORDER_NONE);
    buttons->Add(playBtn, 0, wxALL, 5);
    
    sizer->Add(buttons, 0, wxALL, 5);
    
    playBtn->Connect(playBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                    wxCommandEventHandler(GamesPanel::onPlay), NULL, this);
    
    SetSizer(sizer);
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::populateList()
{
    wxString path = m_dir_picker->GetPath();
    m_item_list->ClearAll();
        
    if (path.IsEmpty()) return;
    
    wxArrayString columns;          std::vector<int> sizes;
    columns.Add( _("File Name") );  sizes.push_back( 300 );
    columns.Add( _("Good Name") );  sizes.push_back( 225 );
    columns.Add( _("Country") );    sizes.push_back( 75 );
    columns.Add( _("Size") );       sizes.push_back( 75 );
    
    const int count = columns.Count();
    for (int n=0; n<count; n++)
    {
        wxListItem col;
        col.SetId(n);
        col.SetText( columns[n] );
        col.SetWidth( sizes[n] );
        m_item_list->InsertColumn(n, col);
    }
    
    std::vector<RomInfo> roms = getRomsInDir(path);
    
    const int item_amount = roms.size();
    for (int n=0; n<item_amount; n++)
    {
        RomInfo& curritem = roms[n];
                
        wxListItem item;
        item.SetId(n);
        item.SetText( curritem.m_file_name );
        
        m_item_list->InsertItem( item );
        
        // set value in first column
        m_item_list->SetItem(n, 0, curritem.m_file_name);
        
        // set value in second column
        m_item_list->SetItem(n, 1, "..."); // TODO: pass real "good name" value
        
        // set value in third column
        m_item_list->SetItem(n, 2, "JA"); // TODO: pass real "Country" value
        
        // set value in fourth column
        m_item_list->SetItem(n, 3, "128 MiB"); // TODO: pass real "Size" value
    }
}

// -----------------------------------------------------------------------------------------------------------

GamesPanel::~GamesPanel()
{
}

// -----------------------------------------------------------------------------------------------------------

std::vector<GamesPanel::RomInfo> GamesPanel::getRomsInDir(wxString dirpath)
{
    std::vector<RomInfo> out;
    
    wxArrayString children;
    const int count = wxDir::GetAllFiles(dirpath, &children);
    
    assert(count == (int)children.Count());
    
    for (int n=0; n<count; n++)
    {
        wxString filename = wxFileName::FileName(children[n]).GetFullName();
        
        // roms typically end with "*.n64|*.z64|*.v64"
        if (filename.EndsWith("64") && !filename.StartsWith("."))
        {
            out.push_back( RomInfo(children[n],
                                   filename) );
        }
    }
    
    return out;
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPathChange(wxFileDirPickerEvent& event)
{
    populateList();
}

// -----------------------------------------------------------------------------------------------------------

void GamesPanel::onPlay(wxCommandEvent& evt)
{
    // TODO: add some kind of progress indicator, launching ROM can take a little while
    
    wxString path = m_dir_picker->GetPath();        
    if (path.IsEmpty())
    {
        wxBell();
        return;
    }
    
    long item = m_item_list->GetNextItem(-1,
                                        wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
    wxString file = path + wxFileName::GetPathSeparator() + m_item_list->GetItemText(item);
    printf("\n==== Running file '%s' ====\n\n", (const char*)file.utf8_str());

    try
    {
        m_api->loadRom(file);
        m_api->runEmulation();
    }
    catch (std::runtime_error& ex)
    {
        wxMessageBox(ex.what());
    }
}

// -----------------------------------------------------------------------------------------------------------
