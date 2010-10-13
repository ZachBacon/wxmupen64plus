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

#include "sdlkeypicker.h"

#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include <wx/button.h>
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

wxSDLKeyPicker::wxSDLKeyPicker(wxWindow* parent, SDLKey key) : wxPanel(parent, wxID_ANY)
{
    m_key = key;
    m_format = FORMAT_KEY_INT;
    m_btn2 = NULL;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn = new wxButton(this, wxID_ANY, "");
#ifdef __WXMAC__
    sizer->AddSpacer(2);
    sizer->Add(m_btn, 1, wxBOTTOM, 5);
    sizer->AddSpacer(2);
#else
    sizer->Add(m_btn, 1);
#endif
    updateLabel();
    
    m_btn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick), NULL, this);
    
    SetMinSize( wxSize(150, -1) );
    SetSizerAndFit(sizer);
}

// -----------------------------------------------------------------------------------------------------------

wxSDLKeyPicker::wxSDLKeyPicker(wxWindow* parent, wxString curr, bool isDouble) : wxPanel(parent, wxID_ANY)
{
    m_format = (isDouble ? FORMAT_DOUBLE_STRING : FORMAT_STRING);
    m_binding = curr;
    m_btn2 = NULL;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn = new wxButton(this, wxID_ANY, "");
#ifdef __WXMAC__
    sizer->AddSpacer(2);
    sizer->Add(m_btn, 1, wxBOTTOM, 5);
    sizer->AddSpacer(2);
#else
    sizer->Add(m_btn, 1);
#endif

    if (m_format == FORMAT_DOUBLE_STRING)
    {
        m_btn2 = new wxButton(this, wxID_ANY, "");
#ifdef __WXMAC__
        sizer->AddSpacer(2);
        sizer->Add(m_btn2, 1, wxBOTTOM, 5);
        sizer->AddSpacer(2);
#else
        sizer->Add(m_btn2, 1);
#endif
    }

    updateLabel();
    
    m_btn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick), NULL, this);
    
    if (m_btn2 != NULL)
    {
        m_btn2->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick), NULL, this);
    }
    
    SetMinSize( wxSize(150, -1) );
    SetSizerAndFit(sizer);
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::onClick(wxCommandEvent& evt)
{
    PressAKey dialog;
    SDLKey key = dialog.getResult();
    if (key != SDLK_UNKNOWN)
    {
        if (m_format == FORMAT_KEY_INT)
        {
            m_key = key;
        }
        else if (m_format == FORMAT_STRING)
        {
            // TODO: also support gamepad bindings
            m_binding = wxString::Format("key(%i)", key);
        }
        else if (m_format == FORMAT_DOUBLE_STRING)
        {
            // TODO: gamepad bindings?
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            
            if (evt.GetId() == m_btn2->GetId())
            {
                key2 = wxString::Format("%i", key);
            }
            else
            {
                key1 = wxString::Format("%i", key);
            }
            
            m_binding = wxString::Format("key(%s,%s)", key1, key2);
        }
        updateLabel();
    }
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::updateLabel()
{
    wxString label;
    
    if (m_format == FORMAT_KEY_INT)
    {
        if (m_key == SDLK_UNKNOWN)
        {
            label = _("Select a key...");
        }
        else
        {
            label = SDL_GetKeyName(m_key);
        }
    }
    else if (m_format == FORMAT_STRING)
    {
        if (m_binding.StartsWith("key("))
        {
            long keyval = -1;
            wxString key = m_binding.AfterFirst('(').BeforeLast(')');
            const bool success = key.ToLong(&keyval);
            if (success)
            {
                label = SDL_GetKeyName((SDLKey)keyval);
            }
            else
            {
                label = m_binding;
            }
        }
        else if (m_binding.StartsWith("button("))
        {
            long btnval = -1;
            wxString btn = m_binding.AfterFirst('(').BeforeLast(')');
            const bool success = btn.ToLong(&btnval);
            if (success)
            {
                label = wxString::Format(_("Button %i"), btnval);
            }
            else
            {
                label = m_binding;
            }
        }
        else
        {
            label = m_binding;
        }
    }
    else if (m_format == FORMAT_DOUBLE_STRING)
    {
        if (m_binding.StartsWith("key("))
        {
            long key1val = -1, key2val = -1;
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            const bool success = (key1.IsEmpty() || key1.ToLong(&key1val)) &&
                                 (key2.IsEmpty() || key2.ToLong(&key2val));
            if (success)
            {
                label = (key1val < 0 ? "" : SDL_GetKeyName((SDLKey)key1val));
                if (key2val >= 0)
                {
                    m_btn2->SetLabel(SDL_GetKeyName((SDLKey)key2val));
                }
            }
            else
            {
                label = m_binding;
                m_btn2->SetLabel("???");
            }
        }
        else if (m_binding.StartsWith("axis("))
        {
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            
            label = key1;
            m_btn2->SetLabel(key2);
        }
        else
        {
            label = m_binding;
        }
    }
    else
    {
        label = "??";
        assert(false);
    }
    
    m_btn->SetLabel(label);
}

// -----------------------------------------------------------------------------------------------------------
