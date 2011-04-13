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

#ifdef __WXMSW__
#include <wx/wx.h>
#endif

#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/msgdlg.h>

#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include "mupen64plusplus/MupenAPI.h"
#include "wxvidext.h"

#include <set>

// FIXME: handle the player closing the frame

std::set<int> pressed_keys;

class BasicGLPane : public wxGLCanvas
{
    wxGLContext*	m_context;
 
public:
	BasicGLPane(wxFrame* parent, int* args);
	virtual ~BasicGLPane();
    
	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
    
    /*
	void render(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    */
    
    /*
	// events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
    */
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
    
    void setCurrent()
    {
        wxGLCanvas::SetCurrent(*m_context);
    }
    
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BasicGLPane, wxGLCanvas)
EVT_SIZE(BasicGLPane::resized)
EVT_KEY_DOWN(BasicGLPane::keyPressed)
EVT_KEY_UP(BasicGLPane::keyReleased)
END_EVENT_TABLE()

int wxToSDL(int code)
{
    switch (code)
    {
        case WXK_BACK:   return SDLK_BACKSPACE;
        case WXK_TAB:    return SDLK_TAB;
        case WXK_RETURN: return SDLK_RETURN;
        case WXK_ESCAPE: return SDLK_ESCAPE;

    /* values from 33 to 126 are reserved for the standard ASCII characters */

    /* values from 128 to 255 are reserved for ASCII extended characters
           (note that there isn't a single fixed standard for the meaning
           of these values; avoid them in portable apps!) */

    /* These are not compatible with unicode characters.
       If you want to get a unicode character from a key event, use
       wxKeyEvent::GetUnicodeKey                                    */
       
        case 'a': case 'A': return SDLK_a;
        case 'b': case 'B': return SDLK_b;
        case 'c': case 'C': return SDLK_c;
        case 'd': case 'D': return SDLK_d;
        case 'e': case 'E': return SDLK_e;
        case 'f': case 'F': return SDLK_f;
        case 'g': case 'G': return SDLK_g;
        case 'h': case 'H': return SDLK_h;
        case 'i': case 'I': return SDLK_i;
        case 'j': case 'J': return SDLK_j;
        case 'k': case 'K': return SDLK_k;
        case 'l': case 'L': return SDLK_l;
        case 'm': case 'M': return SDLK_m;
        case 'n': case 'N': return SDLK_n;
        case 'o': case 'O': return SDLK_o;
        case 'p': case 'P': return SDLK_p;
        case 'q': case 'Q': return SDLK_q;
        case 'r': case 'R': return SDLK_r;
        case 's': case 'S': return SDLK_s;
        case 't': case 'T': return SDLK_t;
        case 'u': case 'U': return SDLK_u;
        case 'v': case 'V': return SDLK_v;
        case 'w': case 'W': return SDLK_w;
        case 'x': case 'X': return SDLK_x;
        case 'y': case 'Y': return SDLK_y;
        case 'z': case 'Z': return SDLK_z;
        
        case '\n':        return SDLK_RETURN;
        case WXK_SPACE:   return SDLK_SPACE;
        case WXK_DELETE:  return SDLK_DELETE;
        //case WXK_START:
        //case WXK_LBUTTON:
        //case WXK_RBUTTON:
        //case WXK_CANCEL:
        //case WXK_MBUTTON:
        case WXK_CLEAR:    return SDLK_CLEAR;
        case WXK_SHIFT:    return SDLK_LSHIFT;
        case WXK_ALT:      return SDLK_LALT;
        case WXK_CONTROL:  return SDLK_LCTRL;
        case WXK_MENU:     return SDLK_MENU;
        case WXK_PAUSE:    return SDLK_PAUSE;
        //case WXK_CAPITAL:
        case WXK_END:      return SDLK_END;
        case WXK_HOME:     return SDLK_HOME;
        case WXK_LEFT:     return SDLK_LEFT;
        case WXK_UP:       return SDLK_UP;
        case WXK_RIGHT:    return SDLK_RIGHT;
        case WXK_DOWN:     return SDLK_DOWN;
        //case WXK_SELECT:
        case WXK_PRINT:    return SDLK_PRINT;
        //case WXK_EXECUTE:
        //case WXK_SNAPSHOT:
        case WXK_INSERT:   return SDLK_INSERT;
        case WXK_HELP:     return SDLK_HELP;
        case WXK_NUMPAD0:  return SDLK_0;
        case WXK_NUMPAD1:  return SDLK_1;
        case WXK_NUMPAD2:  return SDLK_2;
        case WXK_NUMPAD3:  return SDLK_3;
        case WXK_NUMPAD4:  return SDLK_4;
        case WXK_NUMPAD5:  return SDLK_5;
        case WXK_NUMPAD6:  return SDLK_6;
        case WXK_NUMPAD7:  return SDLK_7;
        case WXK_NUMPAD8:  return SDLK_8;
        case WXK_NUMPAD9:  return SDLK_9;
        case WXK_MULTIPLY: return SDLK_ASTERISK;
        case WXK_ADD:      return SDLK_PLUS;
        //case WXK_SEPARATOR:
        case WXK_SUBTRACT: return SDLK_MINUS;
        //case WXK_DECIMAL:
        //case WXK_DIVIDE:
        case WXK_F1:       return SDLK_F1;
        case WXK_F2:       return SDLK_F2;
        case WXK_F3:       return SDLK_F3;
        case WXK_F4:       return SDLK_F4;
        case WXK_F5:       return SDLK_F5;
        case WXK_F6:       return SDLK_F6;
        case WXK_F7:       return SDLK_F7;
        case WXK_F8:       return SDLK_F8;
        case WXK_F9:       return SDLK_F9;
        case WXK_F10:      return SDLK_F10;
        case WXK_F11:      return SDLK_F11;
        case WXK_F12:      return SDLK_F12;
        case WXK_F13:      return SDLK_F13;
        case WXK_F14:      return SDLK_F14;
        case WXK_F15:      return SDLK_F15;
        case WXK_NUMLOCK:  return SDLK_NUMLOCK;
        //case WXK_SCROLL:
        case WXK_PAGEUP:   return SDLK_PAGEUP;
        case WXK_PAGEDOWN: return SDLK_PAGEDOWN;
        
        case WXK_WINDOWS_LEFT: return SDLK_LMETA;
        case WXK_WINDOWS_RIGHT: return SDLK_RMETA;
        case WXK_WINDOWS_MENU: return SDLK_MENU;
        case WXK_COMMAND: return SDLK_LCTRL;
        default: return code;
    }
}

void BasicGLPane::keyPressed(wxKeyEvent& event)
{
    injectKeyEvent(true, wxToSDL(event.GetKeyCode()));
    pressed_keys.insert(event.GetKeyCode());
}

void BasicGLPane::keyReleased(wxKeyEvent& event)
{
    injectKeyEvent(false, wxToSDL(event.GetKeyCode()));
    pressed_keys.erase(event.GetKeyCode());
}


BasicGLPane::BasicGLPane(wxFrame* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	m_context = new wxGLContext(this);
    
    // To avoid flashing on MSW
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}
 
BasicGLPane::~BasicGLPane()
{
	delete m_context;
}
 
void BasicGLPane::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	
    Refresh();
}

int BasicGLPane::getWidth()
{
    return GetSize().x;
}
 
int BasicGLPane::getHeight()
{
    return GetSize().y;
}

wxFrame* frame = NULL;
BasicGLPane* glPane = NULL;
bool fullscreen = false;

bool doublebuffer = true;
int buffersize = 32;
int depthsize = 32;
int redsize = 8;
int greensize = 8;
int bluesize = 8;
int alphasize = 8;

m64p_error VidExt_Init()
{
    printf(">>>>>>>>>>>> WX: VidExt_Init\n");
    return M64ERR_SUCCESS;
}

m64p_error VidExt_Quit()
{
    if (frame)
    {
        frame->Close();
        frame = NULL;
    }
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ListFullscreenModes(m64p_2d_size *SizeArray, int *NumSizes)
{
    printf(">>>>>>>>>>>> WX: VidExt_ListFullscreenModes\n");
    
    // TODO: do better!!
    static m64p_2d_size s[2];
    
    s[0].uiWidth = 640;
    s[0].uiHeight = 480;
    
    s[1].uiWidth = 800;
    s[1].uiHeight = 600;
    
    SizeArray = s;
    *NumSizes = 2;
    
    return M64ERR_SUCCESS;
}

m64p_error VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel, /*m64p_video_mode*/ int ScreenMode)
{
    printf(">>>>>>>>>>>> WX: VidExt_SetVideoMode\n");
    frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Mupen64Plus"), wxPoint(50,50), wxSize(Width,Height));
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	
    /*
        bool doublebuffer = true;
        int buffersize = 32;
        int depthsize = 32;
        int redsize = 8;
        int greensize = 8;
        int bluesize = 8;
        int alphasize = 8;

        WX_GL_RGBA = 1,        // use true color palette (on if no attrs specified)
        WX_GL_BUFFER_SIZE,     // bits for buffer if not WX_GL_RGBA
        WX_GL_LEVEL,           // 0 for main buffer, >0 for overlay, <0 for underlay
        WX_GL_DOUBLEBUFFER,    // use double buffering (on if no attrs specified)
        WX_GL_STEREO,          // use stereoscopic display
        WX_GL_AUX_BUFFERS,     // number of auxiliary buffers
        WX_GL_MIN_RED,         // use red buffer with most bits (> MIN_RED bits)
        WX_GL_MIN_GREEN,       // use green buffer with most bits (> MIN_GREEN bits)
        WX_GL_MIN_BLUE,        // use blue buffer with most bits (> MIN_BLUE bits)
        WX_GL_MIN_ALPHA,       // use alpha buffer with most bits (> MIN_ALPHA bits)
        WX_GL_DEPTH_SIZE,      // bits for Z-buffer (0,16,32)
        WX_GL_STENCIL_SIZE,    // bits for stencil buffer
        WX_GL_MIN_ACCUM_RED,   // use red accum buffer with most bits (> MIN_ACCUM_RED bits)
        WX_GL_MIN_ACCUM_GREEN, // use green buffer with most bits (> MIN_ACCUM_GREEN bits)
        WX_GL_MIN_ACCUM_BLUE,  // use blue buffer with most bits (> MIN_ACCUM_BLUE bits)
        WX_GL_MIN_ACCUM_ALPHA, // use alpha buffer with most bits (> MIN_ACCUM_ALPHA bits)
        WX_GL_SAMPLE_BUFFERS,  // 1 for multisampling support (antialiasing)
        WX_GL_SAMPLES          // 4 for 2x2 antialising supersampling on most graphics cards
        */
    // TODO: make more parameters configurable?
    int args[] = {WX_GL_RGBA, WX_GL_BUFFER_SIZE, buffersize, WX_GL_DOUBLEBUFFER,
                  WX_GL_DEPTH_SIZE, depthsize, /*WX_GL_MIN_RED, redsize,
                  WX_GL_MIN_GREEN, greensize,  WX_GL_MIN_BLUE, bluesize,*/ 0};
    
    if (not wxGLCanvas::IsDisplaySupported(args))
    {
        wxMessageBox( _("Sorry, your system does not support the selected video configuration") );
        return M64ERR_UNSUPPORTED;
    }
    
    glPane = new BasicGLPane(frame, args);
    sizer->Add(glPane, 1, wxEXPAND);
	
    frame->SetSizer(sizer);
	//frame->Center();
    frame->Layout();
    frame->Show();
    
    glPane->setCurrent();
    
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    
    return M64ERR_SUCCESS;
}

m64p_error VidExt_SetCaption(const char *Title)
{
    printf(">>>>>>>>>>>> WX: VidExt_SetCaption : '%s'\n", Title);
    if (frame)
    {
        frame->SetTitle(wxString("WX : ") + wxString::FromAscii(Title));
    }
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ToggleFullScreen()
{
    printf(">>>>>>>>>>>> WX: VidExt_ToggleFullScreen\n");
    
    fullscreen = not fullscreen;
    
    // TODO: toggle the frame if shown
    
    return M64ERR_SUCCESS;
}

void* VidExt_GL_GetProcAddress(const char* Proc)
{
    printf(">>>>>>>>>>>> WX: VidExt_GL_GetProcAddress : '%s'\n", Proc);
    
#ifdef __WXMSW__
    return (void*)wglGetProcAddress(Proc);
#elif __WXGTK__
	return (void*)glXGetProcAddress((const GLubyte*)Proc);
#else
    return SDL_GL_GetProcAddress(Proc);
#endif
}

m64p_error VidExt_GL_SetAttribute(m64p_GLattr Attr, int Value)
{
    printf(">>>>>>>>>>>> WX: VidExt_GL_SetAttribute\n");
    
    switch (Attr)
    {
        case M64P_GL_DOUBLEBUFFER:
            printf("M64P_GL_DOUBLEBUFFER = %i\n", Value);
            doublebuffer = Value;
            break;
        
        case M64P_GL_BUFFER_SIZE:
            printf("M64P_GL_BUFFER_SIZE = %i\n", Value);
            buffersize = Value;
            break;
        
        case M64P_GL_DEPTH_SIZE:
            printf("M64P_GL_DEPTH_SIZE = %i\n", Value);
            depthsize = Value;
            break;
        
        case M64P_GL_RED_SIZE:
            printf("M64P_GL_RED_SIZE = %i\n", Value);
            redsize = Value;
            break;
        
        case M64P_GL_GREEN_SIZE:
            printf("M64P_GL_GREEN_SIZE = %i\n", Value);
            greensize = Value;
            break;
        
        case M64P_GL_BLUE_SIZE:
            printf("M64P_GL_BLUE_SIZE = %i\n", Value);
            bluesize = Value;
            break;
        
        case M64P_GL_ALPHA_SIZE:
            printf("M64P_GL_ALPHA_SIZE = %i\n", Value);
            alphasize = Value;
            break;
        
        case M64P_GL_SWAP_CONTROL:
            printf("M64P_GL_SWAP_CONTROL = %i\n", Value);
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLEBUFFERS:
            printf("M64P_GL_MULTISAMPLEBUFFERS = %i\n", Value);
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLESAMPLES:
            printf("M64P_GL_MULTISAMPLESAMPLES = %i\n", Value);
            // FIXME: what's that?
            break;
    }
    
    return M64ERR_SUCCESS;
}

class DcHolder
{
    wxClientDC* dc;
public:
    DcHolder(wxGLCanvas* canvas)
    {
        dc = new wxClientDC(canvas);
    }
    ~DcHolder()
    {
        delete dc;
    }
};
/** Sometimes key up events can be lost (rarely) so make sure every frame */
void cleanupEvents()
{
    std::set<int>::iterator it;
    for (it=pressed_keys.begin(); it!=pressed_keys.end(); it++)
    {
        if (not wxGetKeyState((wxKeyCode)*it))
        {
            injectKeyEvent(false, wxToSDL(*it));
            pressed_keys.erase(it);
        }
    }
}

DcHolder* holder = NULL;

m64p_error VidExt_GL_SwapBuffers()
{
    if (holder != NULL)
    {
        delete holder;
        glPane->SwapBuffers();
    }
    
    wxYield();
    cleanupEvents();
    SDL_PumpEvents();
    
    holder = new DcHolder(glPane);
    
    return M64ERR_SUCCESS;
}

m64p_error VidExtFuncGLGetAttr(m64p_GLattr Attr, int* value)
{
    switch (Attr)
    {
        case M64P_GL_DOUBLEBUFFER:
            *value = doublebuffer;
            return M64ERR_SUCCESS;
        
        case M64P_GL_BUFFER_SIZE:
            *value = buffersize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_DEPTH_SIZE:
            *value = depthsize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_RED_SIZE:
            *value = redsize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_GREEN_SIZE:
            *value = greensize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_BLUE_SIZE:
            *value = bluesize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_ALPHA_SIZE:
            *value = alphasize;
            return M64ERR_SUCCESS;
        
        case M64P_GL_SWAP_CONTROL:
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLEBUFFERS:
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLESAMPLES:
            // FIXME: what's that?
            break;
    }
    
    return M64ERR_INPUT_INVALID;
}

m64p_error installWxVideoExtension()
{
    printf(">>>>>>>>>>>> WX: installWxVideoExtension\n");
    
    m64p_video_extension_functions f;
    f.Functions            = 0xFFFFFFFF; // FIXME: what's this field? it's not documented
    f.VidExtFuncGLGetProc  = &VidExt_GL_GetProcAddress;
    f.VidExtFuncGLSetAttr  = &VidExt_GL_SetAttribute;
    f.VidExtFuncGLSwapBuf  = &VidExt_GL_SwapBuffers;
    f.VidExtFuncInit       = &VidExt_Init;
    f.VidExtFuncListModes  = &VidExt_ListFullscreenModes;
    f.VidExtFuncQuit       = &VidExt_Quit;
    f.VidExtFuncSetCaption = &VidExt_SetCaption;
    f.VidExtFuncSetMode    = &VidExt_SetVideoMode;
    f.VidExtFuncGLGetAttr  = &VidExtFuncGLGetAttr;
    f.VidExtFuncToggleFS   = &VidExt_ToggleFullScreen;

    return coreOverrideVidExt(&f);
}

