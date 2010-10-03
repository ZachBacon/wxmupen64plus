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

#include <wx/button.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

class wxSDLKeyPicker : public wxButton
{
    enum Format
    {
        /** A keyboard binding only; the SDLKey */
        FORMAT_KEY_INT,
        
        /** A binding string, may be either keyboard or joystick */
        FORMAT_STRING
    };
    
    /** The selected key SDL code, or SDLK_UNKNOWN if nothing is selected (for int key mode) */
    SDLKey m_key;
    
    /** For string binding mode */
    wxString m_binding;
    
    /** Format of this binding picker (int or string) */
    Format m_format;
    
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
     */
    wxSDLKeyPicker(wxWindow* parent, wxString curr);
    
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
        assert(m_format == FORMAT_STRING);
        return m_binding;
    }
};

#endif // SDL_KEY_PICKER_H
