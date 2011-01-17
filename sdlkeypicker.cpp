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
#include "main.h"
#include "mupen64plusplus/MupenAPIpp.h"

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/log.h>
#include <wx/statbmp.h>

// -----------------------------------------------------------------------------------------------------------


#if USE_SDL_KEY_PICKER

// TODO: support hats
class PressAKey
{
public:
    enum ResultType
    {
        KEY,
        GAMEPAD_BUTTON,
        GAMEPAD_AXIS,
        CANCELLED,
        DELETE_BINDING
    };
    
private:
    SDL_Surface* message;
    SDL_Surface* cancel;
    SDL_Surface* erase;
    SDL_Surface* screen;
    
    SDLKey m_result;
    int m_button;
    int m_axis;
    char m_axis_dir;
    
    ResultType m_type;
public:

    PressAKey()
    {
        SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
        SDL_JoystickEventState(SDL_ENABLE);
        
        screen = SDL_SetVideoMode( 550, 200, 32, SDL_SWSURFACE ); 
        
        message = SDL_LoadBMP( datadir + "/presskey.bmp" ); 
        if (message == NULL)
        {
            wxMessageBox(_("Failed to load image, make sure your installation is OK!"));
            m_type = CANCELLED;
            return;
        }
        
        cancel = SDL_LoadBMP( datadir + "/cancel.bmp" ); 
        if (cancel == NULL)
        {
            wxMessageBox(_("Failed to load image, make sure your installation is OK!"));
            m_type = CANCELLED;
            return;
        }
        
        erase = SDL_LoadBMP( datadir + "/erase.bmp" ); 
        if (erase == NULL)
        {
            wxMessageBox(_("Failed to load image, make sure your installation is OK!"));
            m_type = CANCELLED;
            return;
        }
        
        bool done = false;
        
        SDL_Rect dst_msg;
        dst_msg.x = 15;
        dst_msg.y = 25;
            
        SDL_Rect dst_cancel;
        dst_cancel.x = 15;
        dst_cancel.y = dst_msg.y + message->h + 35;
        
        SDL_Rect dst_erase;
        dst_erase.x = 15;
        dst_erase.y = dst_cancel.y + cancel->h + 15;
        
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = 550;
        rect.h = 200;
        
        Uint32 white = SDL_MapRGB(screen->format, 255, 255, 255);
        
        while (not done)
        {
            SDL_FillRect(screen, &rect, white);

            SDL_BlitSurface( message, NULL, screen, &dst_msg );
            SDL_BlitSurface( cancel, NULL, screen, &dst_cancel );
            SDL_BlitSurface( erase, NULL, screen, &dst_erase);
            SDL_Flip( screen );
            SDL_Delay( 100 );
        
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
                {
                    m_type = KEY;
                    m_result = event.key.keysym.sym;
                    done = true;
                }
                else if (event.type == SDL_JOYAXISMOTION)
                {
                    if (abs(event.jaxis.value) > 32767*2/3)
                    {
                        m_type = GAMEPAD_AXIS;
                        m_axis = event.jaxis.axis;
                        m_axis_dir = (event.jaxis.value > 0 ? '+' : '-');
                        done = true;
                    }
                }
                else if (event.type == SDL_JOYBUTTONDOWN)
                {
                    if (event.jbutton.state == SDL_PRESSED)
                    {
                        m_type = GAMEPAD_BUTTON;
                        m_button = event.jbutton.button;
                        done = true;
                    }
                }
                else if (event.type == SDL_MOUSEBUTTONUP)
                {
                    const int x = event.button.x;
                    const int y = event.button.y;
                    
                    if (x > dst_cancel.x and
                        y > dst_cancel.y and
                        x < dst_cancel.x + cancel->w and
                        y < dst_cancel.y + cancel->h)
                    {
                        m_type = CANCELLED;
                        done = true;
                    }
                    else if (x > dst_erase.x and
                             y > dst_erase.y and
                             x < dst_erase.x + erase->w and
                             y < dst_erase.y + erase->h)
                    {
                        m_type = DELETE_BINDING;
                        done = true;
                    }
                }
                else if (event.type == SDL_QUIT)
                {
                    m_type = CANCELLED;
                    done = true;
                }
                
            } // end while SDL_PollEvent
        } // end while not done
        
        SDL_FreeSurface( screen );
        SDL_FreeSurface( message );
        SDL_FreeSurface( cancel );
        SDL_FreeSurface( erase );
        
        // Close the frame
        SDL_Quit();
    }
    
    ~PressAKey()
    {
    }
    
    /** Get the key/button/axis that was selected by the user, or CANCELLED if it was cancelled */
    ResultType getResultType() const
    {
        return m_type;
    }
    
    /**
     * Get the keyboard key selected by the user (only valid if getResultType() returned KEY)
     */
    SDLKey getKey() const
    {
        return m_result;
    }
    
    /**
     * Get the gamepad button selected by the user (only valid if getResultType() returned GAMEPAD_BUTTON)
     */
    int getButton() const
    {
        return m_button;
    }
    
    /**
     * Get the gamepad axis selected by the user (only valid if getResultType() returned GAMEPAD_AXIS)
     */
    int getAxis() const
    {
        return m_axis;
    }
    
    /**
     * Get the gamepad axis direction selected by the user (only valid if getResultType() returned GAMEPAD_AXIS)
     */
    char getAxisDir() const
    {
        return m_axis_dir;
    }
};

#endif

#if USE_WX_KEY_PICKER
// TODO: support hats
class PressAKey : public wxDialog
{
    
public:

    enum ResultType
    {
        KEY,
        GAMEPAD_BUTTON,
        GAMEPAD_AXIS,
        CANCELLED,
        DELETE_BINDING
    };

private:
    SDLKey m_result;
    int m_button;
    int m_axis;
    char m_axis_dir;
    
    ResultType m_type;
    
public:

    void onIdle(wxIdleEvent& evt)
    {
        evt.RequestMore();
                
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
            {
                m_type = KEY;
                m_result = event.key.keysym.sym;
                EndModal( GetReturnCode() );
            }
            else if (event.type == SDL_JOYAXISMOTION)
            {
                if (abs(event.jaxis.value) > 32767*2/3)
                {
                    m_type = GAMEPAD_AXIS;
                    m_axis = event.jaxis.axis;
                    m_axis_dir = (event.jaxis.value > 0 ? '+' : '-');
                    EndModal( GetReturnCode() );
                }
            }
            else if (event.type == SDL_JOYBUTTONDOWN)
            {
                if (event.jbutton.state == SDL_PRESSED)
                {
                    m_type = GAMEPAD_BUTTON;
                    m_button = event.jbutton.button;
                    EndModal( GetReturnCode() );
                }
            }
        }
        
        wxMilliSleep(50);
    }
    
    void onCancel(wxCommandEvent& evt)
    {
        m_type = CANCELLED;
        EndModal( GetReturnCode() );
    }

    void onErase(wxCommandEvent& evt)
    {
        m_type = DELETE_BINDING;
        EndModal( GetReturnCode() );
    }

    PressAKey() : wxDialog(NULL, wxID_ANY, _("Press a key..."), wxDefaultPosition, wxSize(450, 250))
    {
        m_result = SDLK_UNKNOWN;
        m_type = CANCELLED;
        
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Please use your keyboard/gamepad now"),
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
    
    /** Get the key/button/axis that was selected by the user, or CANCELLED if it was cancelled */
    ResultType getResultType() const
    {
        return m_type;
    }
    
    /**
     * Get the keyboard key selected by the user (only valid if getResultType() returned KEY)
     */
    SDLKey getKey() const
    {
        return m_result;
    }
    
    /**
     * Get the gamepad button selected by the user (only valid if getResultType() returned GAMEPAD_BUTTON)
     */
    int getButton() const
    {
        return m_button;
    }
    
    /**
     * Get the gamepad axis selected by the user (only valid if getResultType() returned GAMEPAD_AXIS)
     */
    int getAxis() const
    {
        return m_axis;
    }
    
    /**
     * Get the gamepad axis direction selected by the user (only valid if getResultType() returned GAMEPAD_AXIS)
     */
    char getAxisDir() const
    {
        return m_axis_dir;
    }
};
#endif

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
    
    m_btn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxSDLKeyPicker::onClick),
                   NULL, this);
    
    SetMinSize( wxSize(250, -1) );
    SetSizer(sizer);
}

// -----------------------------------------------------------------------------------------------------------

wxSDLKeyPicker::wxSDLKeyPicker(wxWindow* parent, wxString curr, const ConfigParam& param,
                               bool isAnalogCouple) : wxPanel(parent, wxID_ANY)
{
    m_format = (isAnalogCouple ? FORMAT_ANALOG_COUPLE : FORMAT_DIGITAL);
    m_binding = curr;
    m_btn2 = NULL;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn = new wxButton(this, wxID_ANY, "");

    if (not param.m_icon_1.IsEmpty())
    {
        wxBitmap icon(param.m_icon_1, wxBITMAP_TYPE_ANY);
        if (not icon.IsOk())
        {
            wxLogWarning("Failed to load icon '%s', make sure your installation is OK",
                         (const char*)param.m_icon_1.mb_str());
        }
        else
        {
            wxStaticBitmap* icon_widget = new wxStaticBitmap(this, wxID_ANY, icon);
            sizer->Add(icon_widget);
        }
    }

#ifdef __WXMAC__
    sizer->AddSpacer(2);
    sizer->Add(m_btn, 1, wxBOTTOM, 5);
    sizer->AddSpacer(2);
#else
    sizer->Add(m_btn, 1);
#endif

    if (m_format == FORMAT_ANALOG_COUPLE)
    {
        m_btn2 = new wxButton(this, wxID_ANY, "");
        
        if (not param.m_icon_2.IsEmpty())
        {
            wxBitmap icon(param.m_icon_2, wxBITMAP_TYPE_ANY );
            if (not icon.IsOk())
            {
                wxLogWarning("Failed to load icon '%s', make sure your installation is OK",
                             (const char*)param.m_icon_2.mb_str());
            }
            else
            {
                wxStaticBitmap* icon_widget = new wxStaticBitmap(this, wxID_ANY, icon);
                sizer->Add(icon_widget);
            }
        }
        
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

        // help the sizers give space equally to the two buttons (seems only necessary under wxGTK)
        m_btn->SetMinSize( wxSize(75, -1) );
        m_btn2->SetMinSize( wxSize(75, -1) );
    }
    
    SetMinSize( wxSize(250, -1) );
    SetSizer(sizer);
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::onClick(wxCommandEvent& evt)
{
    PressAKey dialog;
        
    const PressAKey::ResultType type = dialog.getResultType();
    if (type == PressAKey::CANCELLED) return;
    if (type == PressAKey::DELETE_BINDING)
    {
        m_key = SDLK_UNKNOWN;
        m_binding = "";
        updateLabel();
        return;
    }

    if (m_format == FORMAT_KEY_INT)
    {
        if (type == PressAKey::KEY)
        {
            SDLKey key = dialog.getKey();
            if (key != SDLK_UNKNOWN)
            {
                m_key = key;
            }
        }
        else
        {
            wxBell();
        }
    }
    else if (m_format == FORMAT_DIGITAL)
    {
        if (type == PressAKey::KEY)
        {
            SDLKey key = dialog.getKey();
            if (key != SDLK_UNKNOWN)
            {
                m_binding = wxString::Format("key(%i)", key);
            }
        }
        else if (type == PressAKey::GAMEPAD_BUTTON)
        {
            int button = dialog.getButton();
            m_binding = wxString::Format("button(%i)", button);
        }
        else if (type == PressAKey::GAMEPAD_AXIS)
        {
            m_binding = wxString::Format("axis(%i%c)", dialog.getAxis(), dialog.getAxisDir());
        }
        else
        {
             wxBell();
        }
    }
    else if (m_format == FORMAT_ANALOG_COUPLE)
    {
        const bool isBtn2 = evt.GetId() == m_btn2->GetId();
        
        if (type == PressAKey::KEY)
        {
            SDLKey key = dialog.getKey();
            
            wxString type = m_binding.BeforeFirst('(');
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            
            if (isBtn2) key2 = wxString::Format("%i", key);
            else        key1 = wxString::Format("%i", key);

            // Check what type this couple use to be. If the type changed, keep both in sync
            // by clearing the one that hasn't yet been converted to the new type
            if (type != "key")
            {
                if (isBtn2) key1 = "";
                else        key2 = "";
            }
            
            m_binding = wxString::Format("key(%s,%s)", key1, key2);
        }
        else if (type == PressAKey::GAMEPAD_AXIS)
        {
            const int axis = dialog.getAxis();
            const char dir = dialog.getAxisDir();
            
            wxString type = m_binding.BeforeFirst('(');
            wxString val1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString val2 = m_binding.AfterLast(',').BeforeLast(')');
            
            if (isBtn2) val2 = wxString::Format("%i%c", axis, dir);
            else        val1 = wxString::Format("%i%c", axis, dir);
            
            // Check what type this couple use to be. If the type changed, keep both in sync
            // by clearing the one that hasn't yet been converted to the new type
            if (type != "axis")
            {
                if (isBtn2) val1 = "";
                else        val2 = "";
            }
            
            m_binding = wxString::Format("axis(%s,%s)", val1, val2);
        }
        /*
        else if (type == PressAKey::GAMEPAD_BUTTON)
        {
            SDLKey key = dialog.getButton();
            
            wxString type = m_binding.BeforeFirst('(');
            wxString btn1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString btn2 = m_binding.AfterLast(',').BeforeLast(')');
            
            if (isBtn2) btn2 = wxString::Format("%i", key);
            else        btn1 = wxString::Format("%i", key);
            
            // Check what type this couple use to be. If the type changed, keep both in sync
            // by clearing the one that hasn't yet been converted to the new type
            if (type != "button")
            {
                if (isBtn2) btn1 = "";
                else        btn2 = "";
            }
            
            m_binding = wxString::Format("button(%s,%s)", btn1, btn2);
        }*/
        else
        {
             wxBell();
        }

    }
    updateLabel();
}

// -----------------------------------------------------------------------------------------------------------

void wxSDLKeyPicker::updateLabel()
{
    wxString label;
    
    if (m_format == FORMAT_KEY_INT)
    {
        if (m_key == SDLK_UNKNOWN)
        {
            label = _("Select a binding...");
        }
        else
        {
            label = SDL_GetKeyName(m_key);
        }
    }
    else if (m_format == FORMAT_DIGITAL)
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
                label = wxString::Format(_("Button %i"), (int)btnval);
            }
            else
            {
                label = m_binding;
            }
        }
        else if (m_binding.StartsWith("axis("))
        {
            long axisval = -1;
            wxString axis = m_binding.AfterFirst('(').BeforeLast(')');
            const bool success = axis.ToLong(&axisval);
            if (success)
            {
                label = wxString::Format(_("Axis %i"), (int)axisval);
            }
            else
            {
                label = m_binding;
            }
        }
        else if (m_binding.IsEmpty())
        {
            label = _("Select a binding...");
        }
        else
        {
            label = m_binding;
        }
    }
    else if (m_format == FORMAT_ANALOG_COUPLE)
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
