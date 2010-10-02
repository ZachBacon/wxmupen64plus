#include "parameterpanel.h"
#include "mupen64plusplus/MupenAPIpp.h"

#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/cshelp.h>

#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

class PressAKey : public wxDialog
{
    SDLKey m_result;
public:

    void onIdle(wxIdleEvent& evt)
    {
        evt.RequestMore();
                
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
            {
                m_result = event.key.keysym.sym;
                EndModal( GetReturnCode() );
            }
        }
    }
    
    void onCancel(wxCommandEvent& evt)
    {
        // m_result is probably already SDLK_UNKNOWN but let's play on the safe side...
        m_result = SDLK_UNKNOWN;
        
        EndModal( GetReturnCode() );
    }

    PressAKey() : wxDialog(NULL, wxID_ANY, _("Press a key..."), wxDefaultPosition, wxSize(450, 250))
    {
        m_result = SDLK_UNKNOWN;
        
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Please press a keyboard key now"),
                                               wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
                                               
        sizer->AddStretchSpacer();
        sizer->Add(label, 0, wxEXPAND | wxALL, 5);
        sizer->AddStretchSpacer();
        
        wxButton* cancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
        sizer->Add(cancel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        cancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PressAKey::onCancel), NULL, this);
        
        // TODO: make this buttons work
        wxButton* erase = new wxButton(this, wxID_CANCEL, _("Erase this key binding"));
        sizer->Add(erase, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        
        sizer->AddSpacer(25);
        
        Connect(wxEVT_IDLE, wxIdleEventHandler(PressAKey::onIdle), NULL, this);
        
        SetSizer(sizer);
        Center();
        ShowModal();
        /*
        printf("Will poll SDL...\n");
        
        SDL_Event event;
        while(SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_KEYDOWN)
            {
                // TODO...
                EndModal( GetReturnCode() );
            }
        }*/
        
    }
    
    /**
     * Get the key selected by the user, or SDLK_UNKNOWN if the dialog was cancelled.
     */
    SDLKey getResult() const
    {
        return m_result;
    }
};

class wxSDLKeyPicker : public wxButton
{
    /** The selected key SDL code, or SDLK_UNKNOWN if nothing is selected */
    SDLKey m_key;
    
public:

    void onClick(wxCommandEvent& evt)
    {
        PressAKey dialog;
        SDLKey key = dialog.getResult();
        if (key != SDLK_UNKNOWN)
        {
            m_key = key;
            updateLabel();
        }
    }
    
    void updateLabel()
    {
        wxString label;
        if (m_key == SDLK_UNKNOWN)
        {
            label = _("Select a key...");
        }
        else
        {
            label = SDL_GetKeyName(m_key);
        }
        
        SetLabel(label);
    }

    /**
     * @param key currently assigned SDL key, or SDLK_UNKNOWN if none
     */
    wxSDLKeyPicker(wxWindow* parent, SDLKey key) : wxButton(parent, wxID_ANY, "")
    {
        m_key = key;
        updateLabel();
        Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick), NULL, this);
    }
    
    SDLKey getKey() const
    {
        return m_key;
    }
};

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

        switch (section.m_parameters[p].m_param_type)        {            case M64TYPE_INT:
            {
                int currVal = section.m_parameters[p].getIntValue();
                
                // FIXME: find better way to identify SDL key parameters
                if (wxString(section.m_parameters[p].m_param_name).StartsWith("Kbd Mapping"))
                {
                    ctrl = new wxSDLKeyPicker(this, (SDLKey)currVal);
                    sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);
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
                        
                        // the user data pointer is abused to contain an int (FIXME)
                        const int index = choice->Append(section.m_parameters[p].m_choices[n].m_name,
                                                         (void*)choiceVal);
                                                         
                        if (choiceVal == currVal)
                        {
                            selection = index;
                        }
                    }
                    
                    choice->SetSelection(selection);
                    
                    ctrl = choice;
                    sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }
                else
                {
                    ctrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%i", currVal),
                                          wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS,
                                          0 /* min */, 99999 /* max */);
                    sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);
                }                break;
            }
                            case M64TYPE_FLOAT:
            {
                float currVal = section.m_parameters[p].getFloatValue();
                ctrl = new wxSpinCtrlDouble(this, wxID_ANY, wxString::Format("%f", currVal),
                                            wxDefaultPosition, wxSize(100, -1));
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }
                        case M64TYPE_BOOL:
            {
                bool currVal = section.m_parameters[p].getBoolValue();
                wxCheckBox* cbox = new wxCheckBox(this, wxID_ANY, "");
                cbox->SetValue(currVal);
                ctrl = cbox;
                sizer->Add(ctrl, 0, wxALIGN_CENTER_VERTICAL  | wxALL, 5);                break;
            }
                        case M64TYPE_STRING:
            {
                std::string currVal = section.m_parameters[p].getStringValue();
                ctrl = new wxTextCtrl(this, wxID_ANY, currVal, wxDefaultPosition, wxSize(150, -1));
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }
                
            default:
            {
                assert(false);
                ctrl = new wxStaticText(this, wxID_ANY, "[???]");
                sizer->Add(ctrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                break;
            }        }
        
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

ParameterPanel::~ParameterPanel()
{
    m_magic_number = 0xDEADBEEF;
}

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
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), ctrl->GetValue());
                    param->setIntValue(ctrl->GetValue());
                }
                else if (dynamic_cast<wxChoice*>(m_parameter_widgets[n]) != NULL)
                {
                    wxChoice* ctrl = (wxChoice*)m_parameter_widgets[n];
                    const int value = (int)ctrl->GetClientData( ctrl->GetSelection() );
                    
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), value);
                    param->setIntValue(value);
                }
                else if (dynamic_cast<wxSDLKeyPicker*>(m_parameter_widgets[n]) != NULL)
                {
                    wxSDLKeyPicker* ctrl = (wxSDLKeyPicker*)m_parameter_widgets[n];
                    printf("[int] parameter %s has value %i\n", param->m_param_name.c_str(), ctrl->getKey());
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
                printf("[float] parameter %s has value %f\n", param->m_param_name.c_str(), ctrl->GetValue());
                param->setFloatValue(ctrl->GetValue());
                break;
            }
                
            case M64TYPE_BOOL:
            {
                wxCheckBox* ctrl = (wxCheckBox*)m_parameter_widgets[n];
                printf("[bool] parameter %s has value %s\n", param->m_param_name.c_str(), ctrl->IsChecked() ? "true" : "false");
                param->setBoolValue(ctrl->IsChecked() ? 1 : 0);
                break;
            }
            
            case M64TYPE_STRING:
            {
                wxTextCtrl* ctrl = (wxTextCtrl*)m_parameter_widgets[n];
                printf("[string] parameter %s has value %s\n", param->m_param_name.c_str(), (const char*)ctrl->GetValue().mb_str());
                param->setStringValue((const char*)ctrl->GetValue().mb_str());
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

void ParameterGroupsPanel::commitNewValues()
{
    assert(m_magic_number == 0x12345678);
    
    const int count = m_panels.size();
    for (int n=0; n<count; n++)
    {
        m_panels[n]->commitNewValues();
    }
}
