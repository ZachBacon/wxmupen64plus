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

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/log.h>
#include <wx/statbmp.h>
#include <wx/event.h>

// -----------------------------------------------------------------------------------------------------------

SDLKey wxKeyToSDL(int wxkey)
{
    switch (wxkey)
    {
        case WXK_BACK: return SDLK_BACKSPACE;
        case WXK_TAB: return SDLK_TAB;
        case WXK_RETURN: return SDLK_RETURN;
        case WXK_ESCAPE: return SDLK_ESCAPE;
        case WXK_SPACE: return SDLK_SPACE;
        case WXK_DELETE: return SDLK_DELETE;
        //case: //WXK_START: ?
        //case: //WXK_LBUTTON: ?
        //case: //WXK_RBUTTON: ?
        //case: //WXK_CANCEL: ?
        //case: //WXK_MBUTTON: ?
        case WXK_CLEAR: return SDLK_CLEAR;
        case WXK_SHIFT: return SDLK_LSHIFT; // FIXME: tell apart left shift from right shift
        case WXK_ALT: return SDLK_LALT; // FIXME: tell apart left alt from right alt
        case WXK_CONTROL: return SDLK_LCTRL; // FIXME: tell apart left control from right control
        case WXK_MENU: return SDLK_MENU;
        case WXK_PAUSE: return SDLK_PAUSE;
        //case: //WXK_CAPITAL: ?
        case WXK_END: return SDLK_END;
        case WXK_HOME: return SDLK_HOME;
        case WXK_LEFT: return SDLK_LEFT;
        case WXK_UP: return SDLK_UP;
        case WXK_RIGHT: return SDLK_RIGHT;
        case WXK_DOWN: return SDLK_DOWN;
        //case: //WXK_SELECT: ?
        case WXK_PRINT: return SDLK_PRINT;
        //case: //WXK_EXECUTE: ?
        //case: //WXK_SNAPSHOT: ?
        case WXK_INSERT: return SDLK_INSERT;
        case WXK_HELP: return SDLK_HELP;
        case WXK_NUMPAD0: return SDLK_0;
        case WXK_NUMPAD1: return SDLK_1;
        case WXK_NUMPAD2: return SDLK_2;
        case WXK_NUMPAD3: return SDLK_3;
        case WXK_NUMPAD4: return SDLK_4;
        case WXK_NUMPAD5: return SDLK_5;
        case WXK_NUMPAD6: return SDLK_6;
        case WXK_NUMPAD7: return SDLK_7;
        case WXK_NUMPAD8: return SDLK_8;
        case WXK_NUMPAD9: return SDLK_9;
        case WXK_MULTIPLY: return SDLK_ASTERISK;
        case WXK_ADD: return SDLK_PLUS;
        //case: //WXK_SEPARATOR: ?
        case WXK_SUBTRACT: return SDLK_MINUS;
        //case: //WXK_DECIMAL: ?
        case WXK_DIVIDE: return SDLK_SLASH;
        case WXK_F1: return SDLK_F1;
        case WXK_F2: return SDLK_F2;
        case WXK_F3: return SDLK_F3;
        case WXK_F4: return SDLK_F4;
        case WXK_F5: return SDLK_F5;
        case WXK_F6: return SDLK_F6;
        case WXK_F7: return SDLK_F7;
        case WXK_F8: return SDLK_F8;
        case WXK_F9: return SDLK_F9;
        case WXK_F10: return SDLK_F10;
        case WXK_F11: return SDLK_F11;
        case WXK_F12: return SDLK_F12;
        case WXK_F13: return SDLK_F13;
        case WXK_F14: return SDLK_F14;
        case WXK_F15: return SDLK_F15;
        case WXK_NUMLOCK: return SDLK_NUMLOCK;
        case WXK_SCROLL: return SDLK_SCROLLOCK;
        case WXK_PAGEUP: return SDLK_PAGEUP;
        case WXK_PAGEDOWN: return SDLK_PAGEDOWN;
        //case WXK_NUMPAD_SPACE: ?
        //case WXK_NUMPAD_TAB: ?
        case WXK_NUMPAD_ENTER: return SDLK_KP_ENTER;
        /*
        case: //WXK_NUMPAD_F1:
        case: //WXK_NUMPAD_F2:
        case: //WXK_NUMPAD_F3:
        case: //WXK_NUMPAD_F4:
        case: //WXK_NUMPAD_HOME:
        case: //WXK_NUMPAD_LEFT:
        case: //WXK_NUMPAD_UP: ?
        case: //WXK_NUMPAD_RIGHT: ?
        case: //WXK_NUMPAD_DOWN: ?
        case: //WXK_NUMPAD_PAGEUP: ?
        case: //WXK_NUMPAD_PAGEDOWN: ?
        case: //WXK_NUMPAD_END: ?
        case: //WXK_NUMPAD_BEGIN: ?
        case: //WXK_NUMPAD_INSERT: ?
        case: //WXK_NUMPAD_DELETE: ?
         * */
        case WXK_NUMPAD_EQUAL: return SDLK_KP_EQUALS;
        case WXK_NUMPAD_MULTIPLY: return SDLK_KP_MULTIPLY;
        case WXK_NUMPAD_ADD: return SDLK_KP_PLUS;
        //case WXK_NUMPAD_SEPARATOR: ?
        case WXK_NUMPAD_SUBTRACT: return SDLK_KP_MINUS;
        case WXK_NUMPAD_DECIMAL: return SDLK_KP_PERIOD;
        case WXK_NUMPAD_DIVIDE: return SDLK_KP_DIVIDE;
        case WXK_WINDOWS_LEFT: return SDLK_LMETA;
        case WXK_WINDOWS_RIGHT: return SDLK_RMETA;
        case WXK_WINDOWS_MENU: return SDLK_MENU;
        case WXK_COMMAND: return SDLK_LMETA;
        case '=': return SDLK_EQUALS;
        case ':': return SDLK_COLON;
        case ';': return SDLK_SEMICOLON;
        case ',': return SDLK_COMMA;
        case '.': return SDLK_PERIOD;
        case '/': return SDLK_SLASH;
        case '\\': return SDLK_BACKSLASH;
        case '`': return SDLK_BACKQUOTE;
        case '&': return SDLK_AMPERSAND;
        case '"': return SDLK_QUOTE;
        case '-': return SDLK_MINUS;
        case '[': return SDLK_LEFTBRACKET;
        case ']': return SDLK_RIGHTBRACKET;
        case '(': return SDLK_LEFTPAREN;
        case ')': return SDLK_RIGHTPAREN;
        default:
            // ASCII
            if (wxkey >= 'a' and wxkey <= 'z') return (SDLKey)wxkey;
            if (wxkey >= 'A' and wxkey <= 'Z') return (SDLKey)(wxkey + 'a' - 'A');
            if (wxkey >= '0' and wxkey <= '9') return (SDLKey)wxkey;
            return SDLK_UNKNOWN;
    }
}

/** Needed because SDL_GetKeyName requires SDL to be initialised, and SDL is not initialised here. */
const char* GetSDLKeyName(SDLKey key)
{
	switch (key)
	{
		case SDLK_BACKSPACE: return "Backspace";
		case SDLK_TAB: return "Tab";
		case SDLK_CLEAR: return "Clear";
		case SDLK_RETURN: return "Return";
		case SDLK_PAUSE: return "Pause";
		case SDLK_ESCAPE: return "Escape";
		case SDLK_SPACE: return "Space";
		case SDLK_EXCLAIM: return "!";
		case SDLK_QUOTEDBL: return "\"";
		case SDLK_HASH: return "Hash";
		case SDLK_DOLLAR:return "$";
		case SDLK_AMPERSAND: return "&";
		case SDLK_QUOTE: return "'";
		case SDLK_LEFTPAREN: return "(";
		case SDLK_RIGHTPAREN: return ")";
		case SDLK_ASTERISK: return "*";
		case SDLK_PLUS: return "+";
		case SDLK_COMMA: return ",";
		case SDLK_MINUS: return "-";
		case SDLK_PERIOD: return ".";
		case SDLK_SLASH: return "/";
		case SDLK_0: return "0";
		case SDLK_1: return "1";
		case SDLK_2: return "2";
		case SDLK_3: return "3";
		case SDLK_4: return "4";
		case SDLK_5: return "5";
		case SDLK_6: return "6";
		case SDLK_7: return "7";
		case SDLK_8: return "8";
		case SDLK_9: return "9";
		case SDLK_COLON: return ":";
		case SDLK_SEMICOLON: return ";";
		case SDLK_LESS: return "<";
		case SDLK_EQUALS: return "=";
		case SDLK_GREATER: return ">";
		case SDLK_QUESTION: return "?";
		case SDLK_AT: return "@";

		case SDLK_LEFTBRACKET: return "[";
		case SDLK_BACKSLASH: return "\\";
		case SDLK_RIGHTBRACKET: return "]";
		case SDLK_UNDERSCORE: return "_";
		case SDLK_BACKQUOTE: return "`";
		
		case 97:
		case 98:
		case 99:
		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:
		case 120:
		case 121:
		case 122:
		{
			static char ascii[2];
			ascii[0] = key;
			ascii[1] = 0;
			return ascii;
		}

		case SDLK_DELETE: return "delete";

		case SDLK_KP0: return "Numpad 0";
		case SDLK_KP1: return "Numpad 1";
		case SDLK_KP2: return "Numpad 2";
		case SDLK_KP3: return "Numpad 3";
		case SDLK_KP4: return "Numpad 4";
		case SDLK_KP5: return "Numpad 5";
		case SDLK_KP6: return "Numpad 6";
		case SDLK_KP7: return "Numpad 7";
		case SDLK_KP8: return "Numpad 8";
		case SDLK_KP9: return "Numpad 9";
		case SDLK_KP_PERIOD: return "Numpad .";
		case SDLK_KP_DIVIDE: return "Numpad /";
		case SDLK_KP_MULTIPLY: return "Numpad *";
		case SDLK_KP_MINUS: return "Numpad -";
		case SDLK_KP_PLUS: return "Numpad +";
		case SDLK_KP_ENTER: return "Numpad Enter";
		case SDLK_KP_EQUALS: return "Numpad =";

		case SDLK_UP: return "Up";
		case SDLK_DOWN: return "Down";
		case SDLK_RIGHT: return "Right";
		case SDLK_LEFT: return "Left";
		case SDLK_INSERT: return "Insert";
		case SDLK_HOME: return "Home";
		case SDLK_END: return "End";
		case SDLK_PAGEUP: return "Pageup";
		case SDLK_PAGEDOWN: return "Pagedown";

		case SDLK_F1: return "F1";
		case SDLK_F2: return "F2";
		case SDLK_F3: return "F3";
		case SDLK_F4: return "F4";
		case SDLK_F5: return "F5";
		case SDLK_F6: return "F6";
		case SDLK_F7: return "F7";
		case SDLK_F8: return "F8";
		case SDLK_F9: return "F9";
		case SDLK_F10: return "F10";
		case SDLK_F11: return "F11";
		case SDLK_F12: return "F12";
		case SDLK_F13: return "F13";
		case SDLK_F14: return "F14";
		case SDLK_F15: return "F15";

		case SDLK_RSHIFT: return "Right shift";
		case SDLK_LSHIFT: return "Left Shift";
		case SDLK_RCTRL: return "Right Control";
		case SDLK_LCTRL: return "Left Control";
		case SDLK_RALT: return "Right Alt";
		case SDLK_LALT: return "Left Alt";
		case SDLK_RMETA: return "Right Meta";
		case SDLK_LMETA	: return "Left Meta";
		case SDLK_LSUPER: return "Left Logo";
		case SDLK_RSUPER	: return "Right Logo";

		case SDLK_HELP: return "Help";
		case SDLK_PRINT: return "Print";
		case SDLK_BREAK: return "Break";
		case SDLK_MENU: return "Menu";
		default:
		{
			static char buffer[256];
			sprintf(buffer,"Key %i",key);
			return buffer;
		}
	}
}

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
        SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
        SDL_JoystickEventState(SDL_ENABLE);
        
	    std::vector<SDL_Joystick*> joysticks;
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			 joysticks.push_back(SDL_JoystickOpen(i)); // TODO: also close them on shutdown?
		}
	
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

    PressAKey(wxWindow* parent) : wxDialog(parent, wxID_ANY, _("Press a key..."), wxDefaultPosition, wxSize(450, 250), wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS)
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
        
        erase->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        erase->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(PressAKey::onWxKeyPress), NULL, this);
        
        pane->SetSizer(sizer);
        Center();
        
        pane->SetFocus();
        SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
        
        
        SDL_JoystickEventState(SDL_ENABLE);
        
		for (int i=0; i<SDL_NumJoysticks(); i++) 
		{
			 m_joysticks.push_back(SDL_JoystickOpen(i));
		}
        
        g_have_answer = false;
        Bind(wxEVT_WINDOW_MODAL_DIALOG_CLOSED, &sheetCallback);
        ShowWindowModal();

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
    
    void poll()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED)
            {
                m_type = KEY;
                m_result = event.key.keysym.sym;
                EndModal( GetReturnCode() );
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
                    EndModal( GetReturnCode() );
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
                    EndModal( GetReturnCode() );
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
            EndModal( GetReturnCode() );
        }
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
