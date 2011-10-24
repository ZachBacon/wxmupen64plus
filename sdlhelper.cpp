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

#include "sdlhelper.h"

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include "main.h"

std::map<int, std::string> gamepads; 

const std::map<int, std::string>& getGamepadList()
{
	return gamepads;
}

void SDL_Helper_Start()
{
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    
    // ====================
    // Init Gamepad Support
    mplog_info("SDL_Helper_Start", "%i joysticks were found.\n\n", SDL_NumJoysticks() );
    
    SDL_JoystickEventState(SDL_ENABLE);
    
    if (SDL_NumJoysticks() > 0) mplog_info("SDL_Helper_Start", "The names of the joysticks are:\n");    
    for (int i=0; i<SDL_NumJoysticks(); i++) 
    {
		gamepads[i] = SDL_JoystickName(i);
        mplog_info("SDL_Helper_Start", "    %s\n", SDL_JoystickName(i));
        /* SDL_Joystick *joystick = */ SDL_JoystickOpen(i); // TODO: also close them on shutdown
    }
	
	SDL_Quit();

    printf("\n");
    // ====================
}



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
#ifdef __WXOSX__
        case WXK_CONTROL: return SDLK_LMETA; // FIXME: tell apart left meta from right meta
#else
        case WXK_CONTROL: return SDLK_LCTRL; // FIXME: tell apart left control from right control
#endif
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
#ifdef __WXOSX__
        case WXK_RAW_CONTROL: return SDLK_LCTRL;
#endif
        case WXK_NUMPAD_UP: return SDLK_8;
        case WXK_NUMPAD_LEFT: return SDLK_4;
        case WXK_NUMPAD_DOWN: return SDLK_5;
        case WXK_NUMPAD_RIGHT: return SDLK_6;
        
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
            
            mplog_warning("SDLHelper", "Unknown key %i\n", wxkey);
            
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
