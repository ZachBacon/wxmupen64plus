#include "sdlkeypicker.h"

#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

// -----------------------------------------------------------------------------------------------------------

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

    void onErase(wxCommandEvent& evt)
    {
        // TODO: make the "erase" button actually work
        
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
        
        wxButton* erase = new wxButton(this, wxID_CANCEL, _("Erase this key binding"));
        sizer->Add(erase, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        erase->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PressAKey::onErase), NULL, this);
        
        sizer->AddSpacer(25);
        
        Connect(wxEVT_IDLE, wxIdleEventHandler(PressAKey::onIdle), NULL, this);
        
        SetSizer(sizer);
        Center();
        ShowModal();        
    }
    
    /**
     * Get the key selected by the user, or SDLK_UNKNOWN if the dialog was cancelled.
     */
    SDLKey getResult() const
    {
        return m_result;
    }
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

wxSDLKeyPicker::wxSDLKeyPicker(wxWindow* parent, SDLKey key) : wxButton(parent, wxID_ANY, "")
{
    m_key = key;
    updateLabel();
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick), NULL, this);
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::onClick(wxCommandEvent& evt)
{
    PressAKey dialog;
    SDLKey key = dialog.getResult();
    if (key != SDLK_UNKNOWN)
    {
        m_key = key;
        updateLabel();
    }
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::updateLabel()
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

// -----------------------------------------------------------------------------------------------------------
