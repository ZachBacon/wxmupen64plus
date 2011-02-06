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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <wx/app.h>
#include <wx/string.h>

extern wxString datadir;
extern wxString libs;

class wxFrame;
class wxToolBar;
class wxStatusBar;
class IConfigurationPanel;
class wxBoxSizer;
class wxToolBarToolBase;

#include "ptr_vector.hpp"
#include "mupen64plusplus/MupenAPIpp.h"

DECLARE_EVENT_TYPE(wxMUPEN_RELOAD_OPTIONS, -1);

// application class
class MupenFrontendApp : public wxApp
{
    Mupen64PlusPlus* m_api;
    wxFrame* m_frame;
    wxToolBar* m_toolbar;
    wxStatusBar* m_status_bar;
   
    IConfigurationPanel* m_curr_panel;
    wxBoxSizer* m_sizer;
    
    int m_current_panel;

    ConfigParam* m_gamesPathParam;
    
    ptr_vector<ConfigSection> m_config;
    
    /**
     * Associates a configuration section with its toolbar icon
     * (FIXME: rename this class, what it does is not obvious)
     */
    struct GraphicalSection
    {
        /** The associated toolbar button */
        wxToolBarToolBase* m_tool;
        
        /** 
         * The mupen API config section associated with this button (some buttons may contain more than
         * one config section)
         */
        std::vector<ConfigSection*> m_config;
        
        bool m_is_game_section;
        
        GraphicalSection(wxToolBarToolBase* tool, ConfigSection* config)
        {
            m_tool = tool;
            if (config != NULL) m_config.push_back(config);
            m_is_game_section = false;
        }
        
        GraphicalSection(wxToolBarToolBase* tool, const std::vector<ConfigSection*>& configs) :
            m_config(configs)
        {
            m_tool = tool;
            m_is_game_section = false;
        }
        
        static GraphicalSection createGamesSection(wxToolBarToolBase* tool)
        {
            GraphicalSection g(tool, NULL);
            g.m_is_game_section = true;
            return g;
        }
    };
    
    /** List of configuration sections */
    std::vector<GraphicalSection> m_toolbar_items;

    void setCurrentPanel(int sectionId);

public:
    virtual bool OnInit();

    void shutdown();

    void onClose(wxCloseEvent& evt);
    void onQuitMenu(wxCommandEvent& evt);
    
    bool makeToolbar(int plugins, int selectedSection);
    
    /**
     * Callback invoked when a toolbar item is clicked.
     * The general action to perform when this happens is to change the currently displayed pane
     */
    void onToolbarItem(wxCommandEvent& evt);
    void onActivate(wxActivateEvent& evt);
    virtual bool OnExceptionInMainLoop();
    void onReloadOptionsRequest(wxCommandEvent& evt);
    
    void manualRemoveCurrentPanel();
    void manualReshowCurrentPanel();
};

wxDECLARE_APP(MupenFrontendApp);

#endif
