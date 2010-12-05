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

#include "parameterpanel.h"
#include "mupen64plusplus/MupenAPIpp.h"
#include "mupen64plusplus/osal_preproc.h"

#include "sdlkeypicker.h"
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/cshelp.h>
#include <wx/log.h>
#include <wx/filepicker.h>
#include <wx/dir.h>

#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>
#include <stdexcept>

// TODO: when changing device type (e.g. from gamepad to keyboard), types may need to be changed in all
//       following input buttons, otherwise changing the selected binding won't work


// -----------------------------------------------------------------------------------------------------------

ParameterPanel::ParameterPanel(wxWindow* parent, ConfigSection& section) :
    wxScrolledWindow(parent, wxID_ANY), m_section(section)
{
    m_magic_number = 0xCAFECAFE;
    wxFlexGridSizer* sizer = new wxFlexGridSizer(2 /* columns */, 2 /* hgap */, 6 /* vgap */);
    
    // silly way to leave some space at top
    sizer->AddSpacer(15);
    sizer->AddSpacer(15);
        
    const int count = section.m_parameters.size();
    for (int p=0; p<count; p++)
    {
        //printf("Parameter %i : %s (enabled = %i)\n", p, section.m_parameters[p].m_param_name.c_str(),
        //                                             section.m_parameters[p].m_enabled);
        if (!section.m_parameters[p].m_enabled) continue;

        wxStaticText* label = new wxStaticText(this, wxID_ANY, section.m_parameters[p].m_param_name);
        sizer->Add( label, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 15 );
        wxWindow* ctrl;

        switch (section.m_parameters[p].m_param_type)
        {
            case M64TYPE_INT:
            {
                int currVal = 0;

                try
                {
                    currVal = section.m_parameters[p].getIntValue();
                }
                catch (std::runtime_error& ex)
                {
                    wxLogError("Could not read value of parameter <%s> : %s\n", section.m_parameters[p].m_param_name.c_str(), ex.what());
                }

                if (section.m_parameters[p].m_special_type == KEYBOARD_KEY_INT)
                {
                    ctrl = new wxSDLKeyPicker(this, (SDLKey)currVal);
                    sizer->Add(ctrl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                else if (section.m_parameters[p].m_choices.size() > 0)
                {
                    // if choices are offered, show a combo
 
                    wxChoice* choice = new wxChoice(this, wxID_ANY);
                    
                    int selection = 0;
                    const int count = section.m_parameters[p].m_choices.size();
                    for (int n=0; n<count; n++)
                    {
                        const int choiceVal = section.m_parameters[p].m_choices[n].m_value;
                        
                        // FIXME: the user data pointer is abused to contain an int
                        const int index = choice->Append(section.m_parameters[p].m_choices[n].m_name,
                                                         (void*)choiceVal);
                                                         
                        if (choiceVal == currVal)
                        {
                            selection = index;
                        }
                    }
                    
                    choice->SetSelection(selection);
                    
                    ctrl = choice;
                    sizer->Add(ctrl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                else
                {
                    ctrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%i", currVal),
                                          wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS,
                                          0 /* min */, 99999 /* max */);
                    ctrl->SetMaxSize(wxSize(100, -1));
                    sizer->Add(ctrl, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                break;
            }
                
            case M64TYPE_FLOAT:
            {
                float currVal = 0.0f;
                try
                {                
                    currVal = section.m_parameters[p].getFloatValue();
                }
                catch (std::runtime_error& ex)
                {
                    wxLogError("Could not read value of parameter <%s> : %s\n", section.m_parameters[p].m_param_name.c_str(), ex.what());
                }

                ctrl = new wxSpinCtrlDouble(this, wxID_ANY, wxString::Format("%f", currVal),
                                            wxDefaultPosition, wxSize(100, -1));
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }
            
            case M64TYPE_BOOL:
            {
                bool currVal = false;

                try
                {
                    currVal = section.m_parameters[p].getBoolValue();
                }
                catch (std::runtime_error& ex)
                {
                    wxLogError("Could not read value of parameter <%s> : %s\n", section.m_parameters[p].m_param_name.c_str(), ex.what());
                }
                
                wxCheckBox* cbox = new wxCheckBox(this, wxID_ANY, "");
                cbox->SetValue(currVal);
                ctrl = cbox;
                sizer->Add(ctrl, 0, wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }
            
            case M64TYPE_STRING:
            {
                std::string currVal = "";

                try
                {
                    currVal = section.m_parameters[p].getStringValue();
                }
                catch (std::runtime_error& ex)
                {
                    wxLogError("Could not read value of parameter <%s> : %s\n", section.m_parameters[p].m_param_name.c_str(), ex.what());
                }
                
                if (section.m_parameters[p].m_special_type == BINDING_DIGITAL_STRING)
                {
                    ctrl = new wxSDLKeyPicker(this, wxString(currVal.c_str()), false);
                }
                else if (section.m_parameters[p].m_special_type == BINDING_ANALOG_COUPLE_STRING)
                {
                    ctrl = new wxSDLKeyPicker(this, wxString(currVal.c_str()), true);
                }
                else if (section.m_parameters[p].m_special_type == DIRECTORY)
                {
                    ctrl = new wxDirPickerCtrl(this, wxID_ANY, wxString(currVal.c_str()));
                    ctrl->SetMinSize( wxSize(350, -1) );
                }
				else if (section.m_parameters[p].m_special_type == PLUGIN_FILE)
                {
					// TODO: when the "path" parameter changes, plugin choices need to be updated!
					// TODO: under the "video" parameter, only display DLLs that are video plugins, etc.
					// FIXME: this code is run once for every 'PLUGIN_FILE' type parameter; it could be
					//        run once and for all per 'm_dir'
					
					wxComboBox* combo = new wxComboBox(this, wxID_ANY);
					
					assert(section.m_parameters[p].m_dir != NULL);
					wxString dir = section.m_parameters[p].m_dir->getStringValue();
					if (dir.IsEmpty())
					{
						wxLogWarning("Could not retrieve plugins path");
					}
					else
					{
						wxArrayString choices;
						wxDir::GetAllFiles(dir, &choices, wxString("*") + OSAL_DLL_EXTENSION);
						
						wxArrayString filenames;
						const int count = choices.size();
						for (int n=0; n<count; n++)
						{
							wxFileName f(choices[n]);
							filenames.Add(f.GetFullName());
						}
						
						combo->Append(filenames);
					}
					
					combo->SetValue(currVal.c_str());
					ctrl = combo;
					ctrl->SetMinSize( wxSize(350, -1) );
                }
                else
                {
                    ctrl = new wxTextCtrl(this, wxID_ANY, currVal, wxDefaultPosition, wxSize(150, -1));
                }
                
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);
                break;
            }
                
            default:
            {
                assert(false);
                ctrl = new wxStaticText(this, wxID_ANY, "[???]");
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }
        } // end swicth
        
        label->SetToolTip( section.m_parameters[p].m_help_string );
        ctrl->SetToolTip( section.m_parameters[p].m_help_string );
        ctrl->SetClientData( &m_section.m_parameters[p] );
        m_parameter_widgets.push_back(ctrl);
        
        //wxContextHelpButton* button = new wxContextHelpButton(this, wxID_ANY);
        //sizer->Add(button, 0, wxALL, 5);
    }
    
    SetSizer(sizer);
    FitInside(); // ask the sizer about the needed size
    SetScrollRate(5, 5);
}

// -----------------------------------------------------------------------------------------------------------

ParameterPanel::~ParameterPanel()
{
    m_magic_number = 0xDEADBEEF;
}

// -----------------------------------------------------------------------------------------------------------

#define CHATTY 0

void ParameterPanel::commitNewValues()
{
    assert(m_magic_number == 0xCAFECAFE);
    
    const int count = m_parameter_widgets.size();
    for (int n=0; n<count; n++)
    {
        ConfigParam* param = (ConfigParam*)m_parameter_widgets[n]->GetClientData();
        assert(param != NULL);
        assert(param->ok());
        
        switch (param->m_param_type)
        {
            case M64TYPE_INT:
            {
                if (dynamic_cast<wxSpinCtrl*>(m_parameter_widgets[n]) != NULL)
                {
                    wxSpinCtrl* ctrl = (wxSpinCtrl*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), ctrl->GetValue());
                    #endif
                    
                    param->setIntValue(ctrl->GetValue());
                }
                else if (dynamic_cast<wxChoice*>(m_parameter_widgets[n]) != NULL)
                {
                    wxChoice* ctrl = (wxChoice*)m_parameter_widgets[n];
                    // FIXME: remove this hack (see above for hack start)
                    const int value = (int)((long)ctrl->GetClientData( ctrl->GetSelection() ));
                    
                    #if CHATTY
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), value);
                    #endif
                    
                    param->setIntValue(value);
                }
                else if (dynamic_cast<wxSDLKeyPicker*>(m_parameter_widgets[n]) != NULL)
                {
                    wxSDLKeyPicker* ctrl = (wxSDLKeyPicker*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), ctrl->getKey());
                    #endif
                    
                    param->setIntValue(ctrl->getKey());
                }
                else
                {
                    assert(false);
                }
                break;
            }
            
            case M64TYPE_FLOAT:
            {
                wxSpinCtrlDouble* ctrl = (wxSpinCtrlDouble*)m_parameter_widgets[n];
                
                #if CHATTY
                printf("[float] parameter %s has value %f\n", param->m_param_name.c_str(), ctrl->GetValue());
                #endif
                
                param->setFloatValue(ctrl->GetValue());
                break;
            }
                
            case M64TYPE_BOOL:
            {
                wxCheckBox* ctrl = (wxCheckBox*)m_parameter_widgets[n];
                
                #if CHATTY
                printf("[bool] parameter %s has value %s\n", param->m_param_name.c_str(),
                       ctrl->IsChecked() ? "true" : "false");
                #endif
                
                param->setBoolValue(ctrl->IsChecked() ? 1 : 0);
                break;
            }
            
            case M64TYPE_STRING:
            {
                if (dynamic_cast<wxTextCtrl*>(m_parameter_widgets[n]) != NULL)
                {
                    wxTextCtrl* ctrl = (wxTextCtrl*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[string] parameter %s has value %s\n", param->m_param_name.c_str(),
                           (const char*)ctrl->GetValue().mb_str());
                    #endif
                    
                    param->setStringValue((const char*)ctrl->GetValue().mb_str());
                }
                else if (dynamic_cast<wxSDLKeyPicker*>(m_parameter_widgets[n]) != NULL)
                {
                    wxSDLKeyPicker* ctrl = (wxSDLKeyPicker*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[string] parameter %s has value %s\n", param->m_param_name.c_str(),
                           (const char*)ctrl->getBindingString().mb_str());
                    #endif
                    
                    param->setStringValue((const char*)ctrl->getBindingString().mb_str());
                }
                else if (dynamic_cast<wxDirPickerCtrl*>(m_parameter_widgets[n])  != NULL)
                {
                    wxDirPickerCtrl* ctrl = (wxDirPickerCtrl*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[string] parameter %s has value %s\n", param->m_param_name.c_str(),
                           (const char*)ctrl->GetPath().mb_str());
                    #endif
                    
                    param->setStringValue((const char*)ctrl->GetPath().mb_str());
                }
				else if (dynamic_cast<wxComboBox*>(m_parameter_widgets[n])  != NULL)
                {
                    wxComboBox* ctrl = (wxComboBox*)m_parameter_widgets[n];
                    
                    #if CHATTY
                    printf("[string] parameter %s has value %s\n", param->m_param_name.c_str(),
                           (const char*)ctrl->GetValue().mb_str());
                    #endif
                    
                    param->setStringValue((const char*)ctrl->GetValue().mb_str());
                }
                else
                {
                    assert(false);
                }
                
                break;
            }
            
            default:
            {
                fprintf(stderr, "[ParameterPanel::commitNewValues] Unknown type %i for param %s\n",
                        param->m_param_type, param->m_param_name.c_str());
                assert(false);
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

ParameterGroupsPanel::ParameterGroupsPanel(wxWindow* parent, std::vector<ConfigSection> sections) :
        wxNotebook(parent, wxID_ANY)
{
    m_magic_number = 0x12345678;
    
    const int count = sections.size();
    for (int n=0; n<count; n++)
    {
        ParameterPanel* panel = new ParameterPanel(this, sections[n]);
        this->AddPage(panel, sections[n].m_section_name.c_str());
        m_panels.push_back(panel);
    }
}

// -----------------------------------------------------------------------------------------------------------

void ParameterGroupsPanel::commitNewValues()
{
    assert(m_magic_number == 0x12345678);
    
    const int count = m_panels.size();
    for (int n=0; n<count; n++)
    {
        m_panels[n]->commitNewValues();
    }
}

// -----------------------------------------------------------------------------------------------------------
