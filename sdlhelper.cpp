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

void SDL_Helper_Start()
{
    // (this is needed on OSX for gamepad input configuration to work [SDL_INIT_VIDEO is necessary to init
    // keyboard support],and is needed on Linux for retrieving the name of each key to work)
    // FIXME: SDL_init is also called when starting a game, and also when using the SDL key picker; thus
    //        SDL_init will be called at times where SDL is already inited. I hvae no idea if this is bad
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    
// ====================
    // Init Gamepad Support
    printf("%i joysticks were found.\n\n", SDL_NumJoysticks() );
    
    SDL_JoystickEventState(SDL_ENABLE);
    
    if (SDL_NumJoysticks() > 0) printf("The names of the joysticks are:\n");    
    for (int i=0; i<SDL_NumJoysticks(); i++) 
    {
        printf("    %s\n", SDL_JoystickName(i));
        /* SDL_Joystick *joystick = */ SDL_JoystickOpen(i); // TODO: also close them on shutdown
    }
    printf("\n");
    // ====================
    
}

