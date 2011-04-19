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

#ifndef VIDEO_EXTENSION_H
#define VIDEO_EXTENSION_H

#include "m64p_vidext.h"

m64p_error VidExt_Init(void);
m64p_error VidExt_Quit(void);
m64p_error VidExt_ListFullscreenModes(m64p_2d_size *SizeArray, int *NumSizes);
m64p_error VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel, /*m64p_video_mode*/ int ScreenMode);
m64p_error VidExt_SetCaption(const char *Title);
m64p_error VidExt_ToggleFullScreen(void);
void* VidExt_GL_GetProcAddress(const char* Proc);
m64p_error VidExt_GL_SetAttribute(m64p_GLattr Attr, int Value);
m64p_error VidExt_GL_SwapBuffers(void);

m64p_error installWxVideoExtension();

void VidExt_InitGLCanvas();

#endif // VIDEO_EXTENSION_H
