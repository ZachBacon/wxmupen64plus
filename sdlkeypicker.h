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
