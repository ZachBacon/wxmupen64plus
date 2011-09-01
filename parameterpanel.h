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

#ifndef PARAMETERPANEL_H
#define PARAMETERPANEL_H

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <vector>

#include "mupen64plusplus/MupenAPIpp.h"

class wxFileDirPickerEvent;

class IConfigurationPanel
{
public:
    virtual void commitNewValues(bool onLeaving) = 0;
    virtual void removeMyselfFrom(wxSizer* parentSizer) = 0;
};

class ParameterPanel : public wxScrolledWindow, public IConfigurationPanel
{
    ConfigSection* m_section;
    std::vector<wxWindow*> m_parameter_widgets;
    unsigned long m_magic_number;
    Mupen64PlusPlus* m_api;
    
    void update();
    
public:

	ParameterPanel(wxWindow* parent, Mupen64PlusPlus* api, ConfigSection* section);
	~ParameterPanel();

    virtual void commitNewValues(bool onLeaving);
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
    
    void onPathChanged(wxFileDirPickerEvent& event);
    void onCheckbox(wxCommandEvent& evt);
    
    void onEnterPressed(wxCommandEvent& evt);
    void onFocusLost(wxFocusEvent& evt);
    void onKeyPicked(wxCommandEvent& evt)
    {
        update();
    }
    void onInputDeviceChange(wxCommandEvent& evt)
    {
        update();
    }
};

class ParameterGroupsPanel : public wxNotebook, public IConfigurationPanel
{
    std::vector<ParameterPanel*> m_panels;
    unsigned long m_magic_number;
    
public:

	ParameterGroupsPanel(wxWindow* parent, Mupen64PlusPlus* api, std::vector<ConfigSection*> sections);
    ~ParameterGroupsPanel()
    {
        m_magic_number = 0xDEADBEEF;
    }
    
    virtual void commitNewValues(bool onLeaving);
    virtual void removeMyselfFrom(wxSizer* parentSizer)
    {
        parentSizer->Detach(this);
        this->Destroy();
    }
};

#endif // PARAMETERPANEL_H
