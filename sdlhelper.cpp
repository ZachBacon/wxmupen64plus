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
    printf("%i joysticks were found.\n\n", SDL_NumJoysticks() );
    
    SDL_JoystickEventState(SDL_ENABLE);
    
    if (SDL_NumJoysticks() > 0) printf("The names of the joysticks are:\n");    
    for (int i=0; i<SDL_NumJoysticks(); i++) 
    {
		gamepads[i] = SDL_JoystickName(i);
        printf("    %s\n", SDL_JoystickName(i));
        /* SDL_Joystick *joystick = */ SDL_JoystickOpen(i); // TODO: also close them on shutdown
    }
	
	SDL_Quit();

    printf("\n");
    // ====================
}

