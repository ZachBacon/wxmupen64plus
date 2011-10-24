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
#ifdef WIN32
#include <wx/msw/winundef.h>
#endif

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include "sdlhelper.h"

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/log.h>
#include <wx/statbmp.h>
#include <wx/event.h>

DEFINE_LOCAL_EVENT_TYPE(wxKEY_PICKED);

// -----------------------------------------------------------------------------------------------------------

#if USE_SDL_KEY_PICKER

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

    PressAKey(wxWindow* parent)
    {
        int error = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
        if (error != 0)
        {
            mplog_error("SDLKeyPicker", "SDL Init failed");
            return;
        }
        SDL_JoystickEventState(SDL_ENABLE);
        
	    std::vector<SDL_Joystick*> joysticks;
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			 joysticks.push_back(SDL_JoystickOpen(i)); // TODO: also close them on shutdown?
		}
        
        screen = SDL_SetVideoMode( 550, 200, 32, SDL_SWSURFACE ); 
        if (screen == NULL)
        {
            mplog_error("SDLKeyPicker", "SDL SetVideoMode failed");
            return;
        }
        
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
        
		// Purge the event queue before starting
		SDL_Event event;
		while(SDL_PollEvent(&event)) { }
		        
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
					 mplog_info("KeyPicker", "     Key pressed : %i\n", event.key.keysym.sym);
                    m_type = KEY;
                    m_result = event.key.keysym.sym;
                    done = true;
                }
                else if (event.type == SDL_JOYAXISMOTION)
                {
					 mplog_info("KeyPicker", "     Axis Motion : (%i) %i \n", event.jaxis.axis, event.jaxis.value);
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
						 mplog_info("KeyPicker", "     Gamepad button pressed : %i\n", m_button = event.jbutton.button);
                        m_type = GAMEPAD_BUTTON;
                        m_button = event.jbutton.button;
                        done = true;
                    }
					else
					{
						mplog_info("KeyPicker", "     Gamepad button released : %i\n", m_button = event.jbutton.button);
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
						 mplog_info("KeyPicker", "Cancelled\n");
                        m_type = CANCELLED;
                        done = true;
                    }
                    else if (x > dst_erase.x and
                             y > dst_erase.y and
                             x < dst_erase.x + erase->w and
                             y < dst_erase.y + erase->h)
                    {
						 mplog_info("KeyPicker", "Binding Delete Clicked\n");
                        m_type = DELETE_BINDING;
                        done = true;
                    }
                }
                else if (event.type == SDL_QUIT)
                {
					 mplog_info("KeyPicker", "Cancelled\n");
                    m_type = CANCELLED;
                    done = true;
                }
                
            } // end while SDL_PollEvent
        } // end while not done
		        
        SDL_FreeSurface( screen );
        SDL_FreeSurface( message );
        SDL_FreeSurface( cancel );
        SDL_FreeSurface( erase );
        
		for (int i=0; i<joysticks.size(); i++) 
		{
			 SDL_JoystickClose(joysticks[i]);
		}
		
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

bool g_have_answer = false;
int g_answer;
void sheetCallback(wxWindowModalDialogEvent& evt)
{
    g_answer = evt.GetReturnCode();
    g_have_answer = true;
}

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
    std::vector<SDL_Joystick*> m_joysticks;
    
public:

    PressAKey(wxWindow* parent) : wxDialog(parent, wxID_ANY, _("Press a key..."), wxDefaultPosition, wxSize(450, 250),
                                           wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxSTAY_ON_TOP)
    {
        m_result = SDLK_UNKNOWN;
        m_type = CANCELLED;
        
        wxPanel* pane = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* label = new wxStaticText(pane, wxID_ANY, _("Please use your keyboard/gamepad now"),
                                               wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
        
        sizer->AddStretchSpacer();
        sizer->Add(label, 0, wxEXPAND | wxALL, 5);
        sizer->AddStretchSpacer();
        
        wxButton* cancel = new wxButton(pane, wxID_CANCEL, _("Cancel"));
        sizer->Add(cancel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        cancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PressAKey::onCancel), NULL, this);
        
        wxButton* erase = new wxButton(pane, wxID_CANCEL, _("Erase this key binding"));
        sizer->Add(erase, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
        erase->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PressAKey::onErase), NULL, this);
        
        sizer->AddSpacer(25);
        
        //Connect(wxEVT_IDLE, wxIdleEventHandler(PressAKey::onIdle), NULL, this);
        
        Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        pane->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        pane->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        cancel->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        cancel->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        label->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        label->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        erase->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        erase->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        pane->SetSizer(sizer);
        Center();
        
        #ifdef __WXMSW__
        label->SetFocus();
        #else
        pane->SetFocus();
        #endif
                
        SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
        
        SDL_JoystickEventState(SDL_ENABLE);
        
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			 m_joysticks.push_back(SDL_JoystickOpen(i));
		}
        
        g_have_answer = false;
        Bind(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, &sheetCallback);
#ifdef __WXMAC__
        ShowWindowModal();
#else
        Show();
#endif

        // FIXME: that's ugly :)
        while (not g_have_answer)
        {
            wxYield();
            wxMilliSleep(10);
            poll();
        }
    }
    
    ~PressAKey()
    {
        for (unsigned int i=0; i<m_joysticks.size(); i++) 
		{
			 SDL_JoystickClose(m_joysticks[i]);
		}
		
        // Close the frame
        SDL_Quit();
    }
    
    void onIdle(wxIdleEvent& evt)
    {
        evt.RequestMore();
        poll();
    }
    
    void closeDlg()
    {
#ifdef __WXMAC__
        EndModal( GetReturnCode() );
#else
        Hide();
        g_have_answer = true;
#endif
    }
    
    void poll()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
            {
                m_type = KEY;
                m_result = event.key.keysym.sym;
                closeDlg();
            }
            /*
            else if (event.type == SDL_JOYAXISMOTION)
            {
                //printf("SDL_JOYAXISMOTION\n");
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
                //printf("SDL_JOYBUTTONDOWN\n");                
                if (event.jbutton.state == SDL_PRESSED)
                {
                    m_type = GAMEPAD_BUTTON;
                    m_button = event.jbutton.button;
                    EndModal( GetReturnCode() );
                }
            }*/
        }
        
        SDL_JoystickUpdate();
        
        for (unsigned int i=0; i<m_joysticks.size(); i++) 
		{
            const int count = SDL_JoystickNumAxes(m_joysticks[i]);
            for (int axis=0; axis<count; axis++)
            {
                Sint16 axisVal = SDL_JoystickGetAxis(m_joysticks[i], axis);
                if (abs(axisVal) > 32767*2/3)
                {
                    m_type = GAMEPAD_AXIS;
                    m_axis = axis;
                    m_axis_dir = (axisVal > 0 ? '+' : '-');
                    closeDlg();
                }
            }
            
            const int count2 = SDL_JoystickNumButtons(m_joysticks[i]);
            for (int btn=0; btn<count2; btn++)
            {
                Uint8 btnVal = SDL_JoystickGetButton(m_joysticks[i], btn);
                if (btnVal == SDL_PRESSED)
                {
                    m_type = GAMEPAD_BUTTON;
                    m_button = btn;
                    closeDlg();
                }
            }
        }
        
        wxMilliSleep(50);
    }
    
    void onWxKeyPress(wxKeyEvent& evt)
    {
        m_type = KEY;
        m_result = wxKeyToSDL(evt.GetKeyCode());
        
        if (m_result == SDLK_UNKNOWN)
        {
            wxBell();
        }
        else
        {
            closeDlg();
        }
    }
    
    void onCancel(wxCommandEvent& evt)
    {
        m_type = CANCELLED;
        closeDlg();
    }

    void onErase(wxCommandEvent& evt)
    {
        m_type = DELETE_BINDING;
        closeDlg();
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

wxSDLKeyPicker::wxSDLKeyPicker(wxWindow* parent, wxString curr, const ConfigParam* param,
                               bool isAnalogCouple) : wxPanel(parent, wxID_ANY)
{
    m_format = (isAnalogCouple ? FORMAT_ANALOG_COUPLE : FORMAT_DIGITAL);
    m_binding = curr;
    m_btn2 = NULL;
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn = new wxButton(this, wxID_ANY, "");

    if (not param->m_icon_1.IsEmpty())
    {
        wxBitmap icon(param->m_icon_1, wxBITMAP_TYPE_ANY);
        if (not icon.IsOk())
        {
            wxLogWarning("Failed to load icon '%s', make sure your installation is OK",
                         (const char*)param->m_icon_1.mb_str());
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
        
        if (not param->m_icon_2.IsEmpty())
        {
            wxBitmap icon(param->m_icon_2, wxBITMAP_TYPE_ANY );
            if (not icon.IsOk())
            {
                wxLogWarning("Failed to load icon '%s', make sure your installation is OK",
                             (const char*)param->m_icon_2.mb_str());
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
    PressAKey dialog(wxGetApp().GetTopWindow());
        
    const PressAKey::ResultType type = dialog.getResultType();
    if (type == PressAKey::CANCELLED) return;
    if (type == PressAKey::DELETE_BINDING)
    {
        m_key = SDLK_UNKNOWN;
        m_binding = "";
        updateLabel();
        wxCommandEvent evt(wxKEY_PICKED, GetId());
        evt.SetString(GetLabel());
        AddPendingEvent(evt);
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
            
            #ifdef __WXOSX__
            if (key == SDLK_LMETA or key == SDLK_RMETA or
                key == SDLK_MENU)
            {
                wxMessageBox( _("WARNING, the Command key is known to cause problems, it is recommended to use another key.") );
            }
            #endif
            
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
    
    wxCommandEvent notification(wxKEY_PICKED, GetId());
    notification.SetString(GetLabel());
    AddPendingEvent(notification);
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
            label = GetSDLKeyName(m_key);
        }
    }
    else if (m_format == FORMAT_DIGITAL)
    {
        int count = 0;
        for (wxString::iterator it = m_binding.begin(); it != m_binding.end(); it++)
        {
            if (*it == '(') count++;
        }
        if (count > 1)
        {
            // TODO: handle multiple bindings
            mplog_warning("KeyPicker", "Cannot handle '" + m_binding + "', will strip it down to the first option only\n");
            m_binding = m_binding.BeforeFirst(L')') + ")";
        }
        
        if (m_binding.StartsWith("key("))
        {
            long keyval = -1;
            wxString key = m_binding.AfterFirst('(').BeforeLast(')');
            const bool success = key.ToLong(&keyval);
            if (success)
            {
                label = GetSDLKeyName((SDLKey)keyval);
            }
            else
            {
                mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type for 'FORMAT_DIGITAL' : <" + m_binding + "> (not a number)\n");
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
                mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type for 'FORMAT_DIGITAL' : <" + m_binding + ">\n");
                label = m_binding;
            }
        }
        else if (m_binding.StartsWith("axis("))
        {
            long axisval = -1;
            wxString axis = m_binding.AfterFirst('(').BeforeLast(')').Trim();
            wxString sign = axis[axis.size() - 1];
            axis = axis.SubString(0, axis.size() - 2);
            const bool success = axis.ToLong(&axisval);
            if (success)
            {
                label = wxString::Format(_("Axis %i") + " " + sign, (int)axisval);
            }
            else
            {
                mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type for 'FORMAT_DIGITAL' : <" + m_binding + ">\n");
                label = m_binding;
            }
        }
        else if (m_binding.IsEmpty())
        {
            label = _("Select a binding...");
        }
        else
        {
            mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type for 'FORMAT_DIGITAL' : <" + m_binding + ">\n");
            label = m_binding;
        }
    }
    else if (m_format == FORMAT_ANALOG_COUPLE)
    {
        int count = 0;
        for (wxString::iterator it = m_binding.begin(); it != m_binding.end(); it++)
        {
            if (*it == '(') count++;
        }
        if (count > 1)
        {
            // TODO: handle multiple bindings
            mplog_warning("KeyPicker", "Cannot handle '" + m_binding + "', will strip it down to the first option only\n");
            m_binding = m_binding.BeforeFirst(L')') + ")";
        }
        
        if (m_binding.StartsWith("key("))
        {
            long key1val = -1, key2val = -1;
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            
            mplog_info("SdlKeyPicker", "Binding : <" + m_binding + ">\n");
            
            const bool success = (key1.IsEmpty() || key1.ToLong(&key1val)) &&
                                 (key2.IsEmpty() || key2.ToLong(&key2val));
            if (success)
            {
                label = (key1val < 0 ? "" : GetSDLKeyName((SDLKey)key1val));
                if (key2val >= 0)
                {
                    m_btn2->SetLabel(GetSDLKeyName((SDLKey)key2val));
                }
            }
            else
            {
                label = m_binding;
                mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding format for 'FORMAT_ANALOG_COUPLE' : <" + m_binding + ">\n");
                m_btn2->SetLabel("???");
            }
        }
        else if (m_binding.StartsWith("axis("))
        {
            wxString key1 = m_binding.AfterFirst('(').BeforeLast(',');
            wxString key2 = m_binding.AfterLast(',').BeforeLast(')');
            
            label = wxString::Format(_("Axis %s"), key1);
            m_btn2->SetLabel(wxString::Format(_("Axis %s"), key2));
        }
        else
        {
            if (not m_binding.IsEmpty())
            {
                mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type for 'FORMAT_ANALOG_COUPLE' : <" + m_binding + ">\n");
            }
            label = m_binding;
        }
    }
    else
    {
        label = "??";
        mplog_warning("SdlKeyPicker.updateLabel", "Unknown binding type format <%i>\n", m_format);
        assert(false);
    }
    
    m_btn->SetLabel(label);
}

// -----------------------------------------------------------------------------------------------------------
