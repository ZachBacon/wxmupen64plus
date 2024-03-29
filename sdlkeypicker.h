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

#ifndef SDL_KEY_PICKER_H
#define SDL_KEY_PICKER_H

#include <wx/panel.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

class ConfigParam;

// Use the wx picker on OSX and Windows, the SDL picker elsewhere... I think this will work OK
#if defined(__WXMAC__) || defined(__WXMSW__)
#define USE_SDL_KEY_PICKER 0
#define USE_WX_KEY_PICKER 1
#else
#define USE_SDL_KEY_PICKER 1
#define USE_WX_KEY_PICKER 0
#endif

DECLARE_LOCAL_EVENT_TYPE(wxKEY_PICKED, -1);


class wxButton;

class wxSDLKeyPicker : public wxPanel
{
    enum Format
    {
        /** A keyboard binding only; the SDLKey */
        FORMAT_KEY_INT,
        
        /** A binding string, may be either keyboard or joystick */
        FORMAT_DIGITAL,
        
        /** Two keys */
        FORMAT_ANALOG_COUPLE
    };
    
    /** The selected key SDL code, or SDLK_UNKNOWN if nothing is selected (for int key mode) */
    SDLKey m_key;
    
    /** For string binding mode */
    wxString m_binding;
    
    /** Format of this binding picker (int or string) */
    Format m_format;
    
    wxButton* m_btn;
    
    /** Only used for double-string formatted input */
    wxButton* m_btn2;
    
public:

    /**
     * Event callback
     */
    void onClick(wxCommandEvent& evt);
    
    void updateLabel();
    
    /**
     * Constructor using the keyboard (integer) format
     * @param key currently assigned SDL key, or SDLK_UNKNOWN if none
     */
    wxSDLKeyPicker(wxWindow* parent, SDLKey key);
    
   /**
     * Constructor using the string binding (keyboard or gamepad) format
     * @param curr The current binding
     * @param isAnalogCouple whether this parameter's string contains two values (which also means analog input)
     */
    wxSDLKeyPicker(wxWindow* parent, wxString curr, const ConfigParam* param, bool isAnalogCouple);
    
    /**
     * Get the selected key binding, or SDLK_UNKNOWN if nothing was selected
     * @pre this must be an int key object (created using the ctor that takes a SDLKey parameter)
     */
    SDLKey getKey() const
    {
        assert(m_format == FORMAT_KEY_INT);
        return m_key;
    }
    
    /**
     * Get the string of the selected binding
     * @pre this must be a string binding object (created using the ctor that takes a wxString parameter)
     */
    wxString getBindingString() const
    {
        assert(m_format == FORMAT_DIGITAL || m_format == FORMAT_ANALOG_COUPLE);
        return m_binding;
    }
    
    bool isStringBinding() const
    {
        return (m_format == FORMAT_DIGITAL or m_format == FORMAT_ANALOG_COUPLE);
    }
    
    wxButton* getButton()
    {
        return m_btn;
    }
    
    wxButton* getButton2()
    {
        return m_btn2;
    }
};

#endif // SDL_KEY_PICKER_H
