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

#include "mupen64plusplus/MupenAPI.h"
#include "wxvidext.h"

// FIXME: handle the player closing the frame

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
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
    */
    
    void setCurrent()
    {
        wxGLCanvas::SetCurrent(*m_context);
    }
    
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BasicGLPane, wxGLCanvas)
/*
EVT_MOTION(BasicGLPane::mouseMoved)
EVT_LEFT_DOWN(BasicGLPane::mouseDown)
EVT_LEFT_UP(BasicGLPane::mouseReleased)
EVT_RIGHT_DOWN(BasicGLPane::rightClick)
EVT_LEAVE_WINDOW(BasicGLPane::mouseLeftWindow)
 * */
EVT_SIZE(BasicGLPane::resized)
/*
EVT_KEY_DOWN(BasicGLPane::keyPressed)
EVT_KEY_UP(BasicGLPane::keyReleased)
EVT_MOUSEWHEEL(BasicGLPane::mouseWheelMoved)
EVT_PAINT(BasicGLPane::render)
 */
END_EVENT_TABLE()
 
/*
// some useful events to use
void BasicGLPane::mouseMoved(wxMouseEvent& event) {}
void BasicGLPane::mouseDown(wxMouseEvent& event) {}
void BasicGLPane::mouseWheelMoved(wxMouseEvent& event) {}
void BasicGLPane::mouseReleased(wxMouseEvent& event) {}
void BasicGLPane::rightClick(wxMouseEvent& event) {}
void BasicGLPane::mouseLeftWindow(wxMouseEvent& event) {}
void BasicGLPane::keyPressed(wxKeyEvent& event) {}
void BasicGLPane::keyReleased(wxKeyEvent& event) {}
*/

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
 
/** Inits the OpenGL viewport for drawing in 3D. */
#if 0
void BasicGLPane::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
}
#endif
 
/** Inits the OpenGL viewport for drawing in 2D. */
#if 0
void BasicGLPane::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
#endif

int BasicGLPane::getWidth()
{
    return GetSize().x;
}
 
int BasicGLPane::getHeight()
{
    return GetSize().y;
}
 
/*
void BasicGLPane::render( wxPaintEvent& evt )
{
    if (!IsShown()) return;
    
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this);
	
    glFlush();
    SwapBuffers();
}*/

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
    
    return SDL_GL_GetProcAddress(Proc);
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

DcHolder* holder = NULL;

m64p_error VidExt_GL_SwapBuffers()
{
    if (holder != NULL)
    {
        delete holder;
        glPane->SwapBuffers();
    }
    
    wxYield();
    
    holder = new DcHolder(glPane);
    
    return M64ERR_SUCCESS;
}

m64p_error installWxVideoExtension()
{
    printf(">>>>>>>>>>>> WX: installWxVideoExtension\n");
    
    m64p_video_extension_functions f;
    f.VidExtFuncGLGetProc  = &VidExt_GL_GetProcAddress;
    f.VidExtFuncGLSetAttr  = &VidExt_GL_SetAttribute;
    f.VidExtFuncGLSwapBuf  = &VidExt_GL_SwapBuffers;
    f.VidExtFuncInit       = &VidExt_Init;
    f.VidExtFuncListModes  = &VidExt_ListFullscreenModes;
    f.VidExtFuncQuit       = &VidExt_Quit;
    f.VidExtFuncSetCaption = &VidExt_SetCaption;
    f.VidExtFuncSetMode    = &VidExt_SetVideoMode;
    
    return coreOverrideVidExt(&f);
}

