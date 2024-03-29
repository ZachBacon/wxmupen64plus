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
#include <wx/dataview.h>

#include "mupen64plusplus/MupenAPIpp.h"
#include "parameterpanel.h"

class wxListCtrl;
class wxDirPickerCtrl;
class wxFileDirPickerEvent;
class wxBitmapButton;
class wxStaticText;
class wxListEvent;
class wxGLCanvas;

DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_STATE_CHANGE, -1);
DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_SAVE_SLOT_CHANGE, -1);
DECLARE_LOCAL_EVENT_TYPE(wxMUPEN_CLEANUP_GL_CANVAS, -1);

class GamesPanel : public wxPanel, public IConfigurationPanel, public IEmuStateListener
{
    wxDataViewListCtrl* m_item_list;
    wxDirPickerCtrl* m_dir_picker;
    ConfigParam* m_gamesPathParam;
    wxBoxSizer* m_list_sizer;
    wxGLCanvas* m_canvas;
    wxPanel* m_center_panel;
    
    int m_previous_width, m_previous_height;
    ConfigParam* m_width_param;
    ConfigParam* m_height_param;
    ConfigParam* m_fullscreen_param;
    
    struct RomInfo
    {
        wxString m_file_name;
        wxString m_internal_name;
        wxString m_full_path;
        wxString m_country;
        
        RomInfo(wxString fullpath, wxString filename, wxString internal_name, wxString country)
        {
            m_full_path = fullpath;
            m_file_name = filename;
            m_internal_name = internal_name;
            m_country = country;
        }
    };
    
    /**
     * @return a list of ROMs under the specified directory
     * @note the RomInfo objects returned are not complete, only file name and path are set
     */
    static std::vector<RomInfo> getRomsInDir(wxString dirpath);
    void populateList();
    
    Mupen64PlusPlus* m_api;
    
    wxBitmapButton* m_play_button;
    wxBitmapButton* m_pause_button;
    wxBitmapButton* m_stop_button;
    //wxStaticText* m_status;
    
    wxString m_currently_loaded_rom;
    
    std::vector<RomInfo> m_roms;
    
    /** currently selected column, starting from 0 */
    int m_curr_col;
    
public:

	GamesPanel(wxWindow* parent, Mupen64PlusPlus* api, ConfigParam* gamesPathParam);
	~GamesPanel();

    virtual void commitNewValues(bool onLeave) {}
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
    
    void onPathChange(wxFileDirPickerEvent& event);
    void onPlay(wxCommandEvent& evt);
    void onPause(wxCommandEvent& evt);
    void onStop(wxCommandEvent& evt);
    void wanderingFocus(wxFocusEvent& evt);
	
    void loadRom(wxString name, wxString pathAndFile);
    
    /** Callback from IEmuStateListener */
    virtual void onStateChanged(m64p_emu_state newState);
    
    /** Callback from IEmuStateListener */
    virtual void onSaveSlotChanged(int saveSlot);

    void initGLCanvas();
    void cleanGLCanvas();
    void onCleanGLCanvas(wxCommandEvent& evt)
    {
        cleanGLCanvas();
    }

    void onMupenStateChangeEvt(wxCommandEvent& evt);
    void onMupenSaveSlotChangeEvt(wxCommandEvent& evt);
};

#endif // GAMES_PANEL_H
