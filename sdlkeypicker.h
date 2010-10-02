#ifndef SDL_KEY_PICKER_H
#define SDL_KEY_PICKER_H

#include <wx/button.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

class wxSDLKeyPicker : public wxButton
{
    /** The selected key SDL code, or SDLK_UNKNOWN if nothing is selected */
    SDLKey m_key;
    
public:

    void onClick(wxCommandEvent& evt);
    void updateLabel();
    
    /**
     * @param key currently assigned SDL key, or SDLK_UNKNOWN if none
     */
    wxSDLKeyPicker(wxWindow* parent, SDLKey key);
    
    SDLKey getKey() const
    {
        return m_key;
    }
};

#endif // SDL_KEY_PICKER_H
