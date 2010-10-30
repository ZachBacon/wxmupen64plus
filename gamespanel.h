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

#ifndef GAMES_PANEL_H
#define GAMES_PANEL_H

#include <wx/panel.h>
#include <vector>

#include "mupen64plusplus/MupenAPIpp.h"
#include "parameterpanel.h"

class wxListCtrl;
class wxDirPickerCtrl;
class wxFileDirPickerEvent;

class GamesPanel : public wxPanel, public IConfigurationPanel
{
    wxListCtrl* m_item_list;
    wxDirPickerCtrl* m_dir_picker;
    ConfigParam m_gamesPathParam;
    
    struct RomInfo
    {
        wxString m_file_name;
        wxString m_full_path;
        
        RomInfo(wxString fullpath, wxString filename)
        {
            m_full_path = fullpath;
            m_file_name = filename;
        }
    };
    
    
    static std::vector<RomInfo> getRomsInDir(wxString dirpath);
    void populateList();
    
    Mupen64PlusPlus* m_api;
    
public:

	GamesPanel(wxWindow* parent, Mupen64PlusPlus* api, ConfigParam gamesPathParam);
	~GamesPanel();

    virtual void commitNewValues() {}
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
    
    void onPathChange(wxFileDirPickerEvent& event);
    void onPlay(wxCommandEvent& evt);
};

#endif // GAMES_PANEL_H
