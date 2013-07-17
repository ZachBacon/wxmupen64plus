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

#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/msgdlg.h>

#ifdef __WXMAC__
/** For a weird reasons, under OSX 10.6, combiners are off by default */
#define GL_NV_register_combiners 1
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

// must come after including gl.h so that we can configure OpenGL
#include <wx/glcanvas.h>

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_keysym.h>
#include <SDL_events.h>

#include "mupen64plusplus/MupenAPI.h"
#include "wxvidext.h"
#include "main.h"
#include "sdlhelper.h"

#include <set>

std::set<int> pressed_keys;

wxMutex* g_mutex = NULL;

class BasicGLPane : public wxGLCanvas
{
    wxGLContext*	m_context;
 
public:
	BasicGLPane(wxWindow* parent, int* args);
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

void BasicGLPane::keyPressed(wxKeyEvent& event)
{
    wxMutexLocker locker(*g_mutex);
    injectKeyEvent(true, wxKeyToSDL(event.GetKeyCode()));
    pressed_keys.insert(event.GetKeyCode());
}

void BasicGLPane::keyReleased(wxKeyEvent& event)
{
    wxMutexLocker locker(*g_mutex);
    injectKeyEvent(false, wxKeyToSDL(event.GetKeyCode()));
    pressed_keys.erase(event.GetKeyCode());    
}


BasicGLPane::BasicGLPane(wxWindow* parent, int* args) :
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
	
    glViewport(0, 0, GetSize().x, GetSize().y);    
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

BasicGLPane* glPane = NULL;

wxFrame* fullscreen_frame = NULL;

#ifdef __WXMAC__
// work around wx bug
wxFrame* fullscreen_helper_frame = NULL;
#endif

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
    g_mutex = new wxMutex();
    printf(">>>>>>>>>>>> WX: VidExt_Init\n");
    return M64ERR_SUCCESS;
}

m64p_error VidExt_Quit()
{
    printf(">>>>>>>>>>>> WX: VidExt_Quit\n");
    
    wxCommandEvent evt(wxMUPEN_CLEAN_GL_CANVAS, -1);
    wxGetApp().AddPendingEvent(evt);
    /*
    if (frame)
    {
        frame->Close();
        frame = NULL;
    }
    */
    
    delete g_mutex;
    g_mutex = NULL;
    return M64ERR_SUCCESS;
}

void VidExt_AsyncCleanup()
{
    if (fullscreen_frame != NULL)
    {
#ifdef __WXMAC__
        fullscreen_helper_frame->ShowFullScreen(false);
        fullscreen_helper_frame->Destroy();
        fullscreen_helper_frame = NULL;
#endif

#ifndef __WXMAC__
        fullscreen_frame->ShowFullScreen(false);
#endif
        fullscreen_frame->Destroy();
        fullscreen_frame = NULL;
    }
	
	glPane = NULL;
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

int gWidth;
int gHeight;
int gBitsPerPixel;

#ifdef __WXMSW__
    // FIXME: for some reason, Condition asserts on Windows???
    class Condition
    {
        bool m_val;
        
    public:
        
        Condition()
        {
            m_val = false;
        }
        
        void wait()
        {
            while (not m_val)
            {
                Sleep(100);
            }
			m_val = false;
        }
        
        void signal()
        {
            m_val = true;
        }
    };
#else
    class Condition
    {
        wxMutex m_mutex;
        wxCondition m_condition;
        
    public:
        
        Condition() : m_mutex(), m_condition(m_mutex)
        {
            printf("Is OK : %i\n", m_condition.IsOk());
            // the mutex should be initially locked
            m_mutex.Lock();
        }
        
        void wait()
        {
            m_condition.Wait();
        }
        
        void signal()
        {
            wxMutexLocker lock(m_mutex);
            m_condition.Signal();
        }
    };
#endif

Condition* g_condition = NULL;

wxGLCanvas* VidExt_InitGLCanvas(wxWindow* parent)
{
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
   
#ifdef __WXGTK__
    // Mr Linux is more picky (at least for me, I guess YMMV)
    int args[] = {WX_GL_RGBA,  WX_GL_DOUBLEBUFFER, 0};
#else
    // TODO: make more parameters configurable?
    int args[] = {WX_GL_RGBA, WX_GL_BUFFER_SIZE, buffersize, WX_GL_DOUBLEBUFFER,
                  WX_GL_DEPTH_SIZE, depthsize, /*WX_GL_MIN_RED, redsize,
                  WX_GL_MIN_GREEN, greensize,  WX_GL_MIN_BLUE, bluesize,*/ 0};
#endif

    if (not wxGLCanvas::IsDisplaySupported(args))
    {
        wxMessageBox( _("Sorry, your system does not support the selected video configuration") );
        glPane = NULL;
        return NULL;
    }
    
    if (fullscreen)
    {
#ifdef __WXMAC__
        fullscreen_helper_frame = new wxFrame(NULL, wxID_ANY, "Mupen64Plus");
        fullscreen_helper_frame->ShowFullScreen(true, wxFULLSCREEN_NOMENUBAR);
#endif
        
        fullscreen_frame = new wxFrame(NULL, wxID_ANY, "Mupen64Plus", wxDefaultPosition, wxDefaultSize,
                                       wxSTAY_ON_TOP);//wxSYSTEM_MENU | wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW);
        glPane = new BasicGLPane(fullscreen_frame, args);
        
//#ifdef __WXMAC__
//        fullscreen_frame->Maximize();
//        fullscreen_frame->Show();
//#else
//        fullscreen_frame->ShowFullScreen(true, wxFULLSCREEN_NOMENUBAR);
//#endif
        fullscreen_frame->Show();
        fullscreen_frame->ShowFullScreen(true);
        fullscreen_frame->Raise();
        fullscreen_frame->SetFocus();
        
        glPane->SetFocus();
        return NULL;
    }
    else
    {
        glPane = new BasicGLPane(parent, args);
        return glPane;
    }
}

void VidExt_InitedGLCanvas()
{
    glPane->SetCursor(wxCursor(wxCURSOR_BLANK));
    
    if (g_condition == NULL)
    {
        wxMutexLocker locker(*g_mutex);
        if (g_condition == NULL) g_condition = new Condition();
    }
    g_condition->signal();
}

/*
void VidExt_InitGLCanvas()
{
    frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Mupen64Plus"), wxPoint(50,50), wxSize(gWidth, gHeight));
    frame->SetClientSize(wxSize(gWidth, gHeight)); // we need a size of Width,Height *excluding* the title bar
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	
 #if 0
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
#endif

    // TODO: make more parameters configurable?
    int args[] = {WX_GL_RGBA, WX_GL_BUFFER_SIZE, buffersize, WX_GL_DOUBLEBUFFER,
                  WX_GL_DEPTH_SIZE, depthsize,
                  //WX_GL_MIN_RED, redsize, WX_GL_MIN_GREEN, greensize,  WX_GL_MIN_BLUE, bluesize,
                  0};
    
    if (not wxGLCanvas::IsDisplaySupported(args))
    {
        wxMessageBox( _("Sorry, your system does not support the selected video configuration") );
        frame = NULL;
        glPane = NULL;
        //return M64ERR_UNSUPPORTED;
    }
    
    glPane = new BasicGLPane(frame, args);
    sizer->Add(glPane, 1, wxEXPAND);
	
    frame->SetSizer(sizer);
	//frame->Center();
    frame->Layout();
    frame->Show();
    
    if (g_condition == NULL)
    {
        wxMutexLocker locker(*g_mutex);
        if (g_condition == NULL) g_condition = new Condition();
    }
    g_condition->signal();
}
#endif
*/

m64p_error VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel,
                              /*m64p_video_mode*/ int ScreenMode,
                              /*m64p_video_flags*/ int Flags)
{
    /*
    typedef enum {
      M64VIDEO_NONE = 1,
      M64VIDEO_WINDOWED,
      M64VIDEO_FULLSCREEN
    } m64p_video_mode;

    typedef enum {
      M64VIDEOFLAG_SUPPORT_RESIZING = 1
    } m64p_video_flags;
    */
    
    gWidth = Width;
    gHeight = Height;
    gBitsPerPixel = BitsPerPixel;
    fullscreen = (ScreenMode == M64VIDEO_FULLSCREEN);
    
    printf(">>>>>>>>>>>> WX: VidExt_SetVideoMode\n");
    wxCommandEvent evt(wxMUPEN_INIT_GL_CANVAS, -1);
    wxGetApp().AddPendingEvent(evt);
    
    if (g_condition == NULL)
    {
        wxMutexLocker locker(*g_mutex);
        if (g_condition == NULL) g_condition = new Condition();
    }
    g_condition->wait();
    
    if (glPane == NULL)
	{
		fprintf(stderr, "glPane is NULL, returning M64ERR_UNSUPPORTED\n");
		return M64ERR_UNSUPPORTED;
	}
    glPane->setCurrent();
    
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
    
    // clear both buffers with black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPane->SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT);
    
    return M64ERR_SUCCESS;
}

m64p_error VidExt_SetCaption(const char *Title)
{
    printf(">>>>>>>>>>>> WX: VidExt_SetCaption : '%s'\n", Title);
    // TODO: set caption?
    /*
    if (frame)
    {
        frame->SetTitle(wxString("WX : ") + wxString::FromAscii(Title));
    }
    */
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
    
    void* out = NULL;
    
#ifdef __WXMSW__
    out = (void*)wglGetProcAddress(Proc);
#elif __WXGTK__
	out = (void*)glXGetProcAddress((const GLubyte*)Proc);
#else

    // FIXME: silly way to fix VidExt_GL_GetProcAddress on OSX
    if (strcmp(Proc, "glClientActiveTextureARB") == 0)
    {
        out = (void*)&glClientActiveTextureARB;
    }
#if GL_NV_register_combiners
    else if (strcmp(Proc, "glCombinerParameterfvNV") == 0)
    {
        //#ifdef GL_GLEXT_FUNCTION_POINTERS
        //out = aglGetProcAddress ("glCombinerParameterfvNV");
        //#else
        out = (void*)&glCombinerParameterfvNV;
        //#endif
    }
    else if (strcmp(Proc, "glFinalCombinerInputNV") == 0)
    {
        out = (void*)&glFinalCombinerInputNV;
    }
    else if (strcmp(Proc, "glCombinerOutputNV") == 0)
    {
        //#ifdef GL_GLEXT_FUNCTION_POINTERS
        //out = aglGetProcAddress ("glCombinerOutputNV");
        //#else
        out = (void*)&glCombinerOutputNV;
        //#endif
    }
    else if (strcmp(Proc, "glCombinerInputNV") == 0)
    {
        //#ifdef GL_GLEXT_FUNCTION_POINTERS
        //out = aglGetProcAddress ("glCombinerInputNV");
        //#else
        out = (void*)&glCombinerInputNV;
        //#endif
    }
    else if (strcmp(Proc, "glCombinerParameteriNV") == 0)
    {
        out = (void*)&glCombinerParameteriNV;
    }
#else
#warning "GL_NV_register_combiners is not defined, *NV OpenGL extensions will not be available"
#endif
    else if (strcmp(Proc, "glActiveTextureARB") == 0)
    {
        out = (void*)&glActiveTextureARB;
    }
    else if (strcmp(Proc, "glMultiTexCoord2f") == 0)
    {
        out = (void*)&glMultiTexCoord2f;
    }
    else if (strcmp(Proc, "glMultiTexCoord2fv") == 0)
    {
        out = (void*)&glMultiTexCoord2fv;
    }
    else if (strcmp(Proc, "glDeleteProgramsARB") == 0)
    {
        out = (void*)&glDeleteProgramsARB;
    }  
    else if (strcmp(Proc, "glProgramStringARB") == 0)
    {
        out = (void*)&glProgramStringARB;
    }    
    else if (strcmp(Proc, "glBindProgramARB") == 0)
    {
        out = (void*)&glBindProgramARB;
    }
    else if (strcmp(Proc, "glGenProgramsARB") == 0)
    {
        out = (void*)&glGenProgramsARB;
    }
    else if (strcmp(Proc, "glProgramEnvParameter4fvARB") == 0)
    {
        out = (void*)&glProgramEnvParameter4fvARB;
    }
    else if (strcmp(Proc, "glFogCoordPointerEXT") == 0)
    {
        out = (void*)&glFogCoordPointerEXT;
    }
    else if (strcmp(Proc, "glActiveTexture") == 0)
    {
        out = (void*)&glActiveTexture;
    }
    else
    {
        out = SDL_GL_GetProcAddress(Proc);
    }
#endif

    if (out == NULL)
    {
        mplog_warning("VideoExtension", "WARNING: Cannot obtain address of '%s'\n", Proc);
    }
    
    return out;
}

m64p_error VidExt_GL_SetAttribute(m64p_GLattr Attr, int Value)
{
    //printf(">>>>>>>>>>>> WX: VidExt_GL_SetAttribute\n");
    
    switch (Attr)
    {
        case M64P_GL_DOUBLEBUFFER:
            //printf("M64P_GL_DOUBLEBUFFER = %i\n", Value);
            doublebuffer = Value;
            break;
        
        case M64P_GL_BUFFER_SIZE:
            //printf("M64P_GL_BUFFER_SIZE = %i\n", Value);
            buffersize = Value;
            break;
        
        case M64P_GL_DEPTH_SIZE:
            //printf("M64P_GL_DEPTH_SIZE = %i\n", Value);
            depthsize = Value;
            break;
        
        case M64P_GL_RED_SIZE:
            //printf("M64P_GL_RED_SIZE = %i\n", Value);
            redsize = Value;
            break;
        
        case M64P_GL_GREEN_SIZE:
            //printf("M64P_GL_GREEN_SIZE = %i\n", Value);
            greensize = Value;
            break;
        
        case M64P_GL_BLUE_SIZE:
            //printf("M64P_GL_BLUE_SIZE = %i\n", Value);
            bluesize = Value;
            break;
        
        case M64P_GL_ALPHA_SIZE:
            //printf("M64P_GL_ALPHA_SIZE = %i\n", Value);
            alphasize = Value;
            break;
        
        case M64P_GL_SWAP_CONTROL:
            //printf("M64P_GL_SWAP_CONTROL = %i\n", Value);
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLEBUFFERS:
            //printf("M64P_GL_MULTISAMPLEBUFFERS = %i\n", Value);
            // FIXME: what's that?
            break;
        
        case M64P_GL_MULTISAMPLESAMPLES:
            //printf("M64P_GL_MULTISAMPLESAMPLES = %i\n", Value);
            // FIXME: what's that?
            break;
    }
    
    return M64ERR_SUCCESS;
}

class DcHolder
{
    wxClientDC dc;
public:
    DcHolder(wxGLCanvas* canvas) : dc(canvas)
    {
        //glClear(GL_COLOR_BUFFER_BIT);
    }
    ~DcHolder()
    {
    }
};
/** Sometimes key up events can be lost (rarely) so make sure every frame */
void cleanupEvents()
{
    wxMutexLocker locker(*g_mutex);
    
    std::set<int>::iterator it;
	
#if defined(__WXMSW__)
	// FIXME: wxWidgets bug, enter and arrows aren't processed as a normal events
	if (wxGetKeyState(WXK_RETURN))
	{
		if (pressed_keys.find(WXK_RETURN) == pressed_keys.end())
		{
			injectKeyEvent(true, wxKeyToSDL(WXK_RETURN));
			pressed_keys.insert(WXK_RETURN);
		}
	}
    /*
	else
	{
		if (pressed_keys.find(WXK_RETURN) != pressed_keys.end())
		{
			injectKeyEvent(false, wxKeyToSDL(WXK_RETURN));
			pressed_keys.erase(WXK_RETURN);
		}	
	}
    */
    
    if (wxGetKeyState(WXK_LEFT))
    {
        if (pressed_keys.find(WXK_LEFT) == pressed_keys.end())
		{
			injectKeyEvent(true, wxKeyToSDL(WXK_LEFT));
			pressed_keys.insert(WXK_LEFT);
		}
    }
    if (wxGetKeyState(WXK_RIGHT))
    {
        if (pressed_keys.find(WXK_RIGHT) == pressed_keys.end())
		{
			injectKeyEvent(true, wxKeyToSDL(WXK_RIGHT));
			pressed_keys.insert(WXK_RIGHT);
		}
    }
    if (wxGetKeyState(WXK_UP))
    {
        if (pressed_keys.find(WXK_UP) == pressed_keys.end())
		{
			injectKeyEvent(true, wxKeyToSDL(WXK_UP));
			pressed_keys.insert(WXK_UP);
		}
    }
    if (wxGetKeyState(WXK_DOWN))
    {
        if (pressed_keys.find(WXK_DOWN) == pressed_keys.end())
		{
			injectKeyEvent(true, wxKeyToSDL(WXK_DOWN));
			pressed_keys.insert(WXK_DOWN);
		}
    }
#endif
	
    std::set<int> toRemove;
    
    for (it=pressed_keys.begin(); it!=pressed_keys.end(); it++)
    {
        #if defined(__WXOSX__)
        // work around weird issue on OSX, wxGetKeyState always returns false for these keys
        // here (threading issue? SDL interference?)
        if (*it == WXK_CONTROL or *it == WXK_COMMAND or *it == WXK_ALT or
            *it == WXK_MENU or *it == WXK_RAW_CONTROL or *it == WXK_WINDOWS_LEFT or
            *it == WXK_WINDOWS_MENU or *it == WXK_WINDOWS_RIGHT) continue;
        #endif
        if (not wxGetKeyState((wxKeyCode)*it))
        {
            //printf("auto-releasing %i\n", *it);
            injectKeyEvent(false, wxKeyToSDL(*it));
            /*
            printf("Erasing %i from {", *it);
            for (std::set<int>::iterator it2=pressed_keys.begin(); it2!=pressed_keys.end(); it2++)
            {
                printf("%i, ", *it2);
            }
            printf("END}\n");
             * */
            toRemove.insert(*it);
        }
    }
    
    for (it=toRemove.begin(); it!=toRemove.end(); it++)
    {
        pressed_keys.erase(*it);
    }
}

DcHolder* holder = NULL;

// hack : move mouse once in a while to prevent screensaver from kicking in
int frames = 0;
bool moveMouseUp = false;

m64p_error VidExt_GL_SwapBuffers()
{
    // stop screensaver
    frames++;
    if (frames > 350)
    {
        wxPoint screenMousePos = wxGetMousePosition();
        wxPoint mousePos = glPane->ScreenToClient(screenMousePos);
        if (screenMousePos.y < 1 or not moveMouseUp)
        {
            mousePos.y += 1;
            moveMouseUp = true;
        }
        else
        {
            mousePos.y -= 1;
            moveMouseUp = false;
        }
        
        glPane->WarpPointer(mousePos.x, mousePos.y);
        frames = 0;
    }
    
    wxMutexGuiEnter();
    if (holder != NULL)
    {
        delete holder;
        glPane->SwapBuffers();
    }
    
    //wxYield();
    cleanupEvents();
    SDL_PumpEvents();
    
    holder = new DcHolder(glPane);
    
    wxMutexGuiLeave();
    return M64ERR_SUCCESS;
}

m64p_error VidExt_ResizeWindow(int w, int h)
{
        // TODO
    printf("Resize request : %i %i\n", w, h);
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
    f.VidExtFuncResizeWindow = &VidExt_ResizeWindow;
  
    return coreOverrideVidExt(&f);
}

