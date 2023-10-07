/******************************************************************************

 @File         OGLESHelloAPI_NullWS.cpp

 @Title        OpenGL ES 1.x HelloAPI Tutorial

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     .

 @Description  Basic Tutorial that shows step-by-step how to initialize OpenGL ES
               1.x, use it for drawing a triangle and terminate it.

******************************************************************************/
#include <stdio.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

// No C++ Standard Library or exception handling on Symbian platform
#include <stdio.h>

#include "directfb.h"
#include "directfb_util.h"
#include "dfb_util_testbench.h"
#include "dfb_util.h"



/****************************************************************************
** Declarations
****************************************************************************/
unsigned int WINWIDTH = 1920;
unsigned int WINHEIGHT = 1080;




/****************************************************************************
** Platform specific functions
****************************************************************************/
#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }
class PlatformData
{
public:    

    PlatformData()
    {
        m_bInited = false;
        m_pDFB = NULL;
        m_pDFB_surface = NULL;
        m_pPrimaryLayer = NULL;
    }

    ~PlatformData()
    {
        this->Release();
    }

    IDirectFBSurface * GetDFBSurface()
    {
        if(!this->m_bInited) 
        {
            this->Init();
        }

        return this->m_pDFB_surface;
    }

#if 0 
    // Layer Mode (Fullscreen Mode)      
    bool Init()
    {
        //DFBWindowDescription  desc;

        DFBDisplayLayerConfig dlc;
        DFBSurfaceDescription dsc;
        DFBCHECK(DirectFBInit( 0, NULL ));

        /* create the super interface */
        DFBCHECK(DirectFBCreate( &m_pDFB ));

        m_pDFB->GetDisplayLayer(m_pDFB, DLID_PRIMARY, &m_pPrimaryLayer);
        m_pPrimaryLayer->GetConfiguration(m_pPrimaryLayer,&dlc);

        dlc.buffermode = DLBM_BACKVIDEO; // double buffer in video memory

        m_pPrimaryLayer->SetConfiguration( m_pPrimaryLayer, &dlc );

        /* set our cooperative level to DFSCL_FULLSCREEN for exclusive access to
        the primary layer */
        DFBCHECK(m_pDFB->SetCooperativeLevel( m_pDFB, DFSCL_FULLSCREEN ));


        /* get the primary surface, i.e. the surface of the primary layer we have
        exclusive access to */
        dsc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS| DSDESC_WIDTH |DSDESC_HEIGHT);
        dsc.width  = WINWIDTH;
        dsc.height = WINHEIGHT;
        dsc.caps = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY  |DSCAPS_TRIPLE);

        DFBCHECK(m_pDFB->CreateSurface( m_pDFB, &dsc, &m_pDFB_surface ));   // primary is layer surface 
        /* Set PlatformData is inited*/
        m_bInited = true;  

        return true;
    }

#else

    // Window Mode
    bool Init()
    {

        DFBDisplayLayerConfig dlc;
        DFBWindowDescription win_desc;


        DFBCHECK(DirectFBInit( 0, NULL ));

        /* create the super interface */
        DFBCHECK(DirectFBCreate( &m_pDFB ));

        m_pDFB->GetDisplayLayer(m_pDFB, DLID_PRIMARY, &m_pPrimaryLayer);
        m_pPrimaryLayer->GetConfiguration(m_pPrimaryLayer,&dlc);
        m_pPrimaryLayer->SetCooperativeLevel(m_pPrimaryLayer, DLSCL_ADMINISTRATIVE);


        dlc.buffermode = DLBM_BACKVIDEO; // double buffer in video memory
        dlc.width = WINWIDTH;
        dlc.height = WINHEIGHT;


        m_pPrimaryLayer->SetConfiguration( m_pPrimaryLayer, &dlc );



        win_desc.flags = DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_POSX | DWDESC_POSY;
        win_desc.caps = DWCAPS_DOUBLEBUFFER;
        win_desc.width = WINWIDTH;
        win_desc.height = WINHEIGHT;
        win_desc.posx = 0;
        win_desc.posy = 0;

        m_pPrimaryLayer->CreateWindow(m_pPrimaryLayer, &win_desc, &m_pWindow);

        m_pWindow->SetOpacity(m_pWindow, 0xff);
        m_pWindow->GetSurface(m_pWindow, &m_pDFB_surface);



        /* Set PlatformData is inited*/
        m_bInited = true;  

        return true;
    }

#endif      

    void Release()
    {
        if (m_pWindow != NULL)
        {
            m_pWindow->Release (m_pWindow);
            m_pWindow = NULL;
        }

        if (m_pPrimaryLayer != NULL)
        {
            m_pPrimaryLayer->Release(m_pPrimaryLayer);
            m_pPrimaryLayer = NULL;
        }

        if (m_pDFB != NULL)
        {
            m_pDFB->Release(m_pDFB);
            m_pDFB = NULL;
        }

        m_bInited = false;
    }

private:   

    bool m_bInited;
    IDirectFB *m_pDFB;
    IDirectFBSurface *m_pDFB_surface;
    IDirectFBDisplayLayer *m_pPrimaryLayer;
    IDirectFBWindow * m_pWindow;      
};

PlatformData *g_platform = NULL;

/*!***********************************************************************
@Function		CreateWindow
@Returns		A window handle
@Description	Creates a window that is suitable for OpenVG rendering
*************************************************************************/
EGLNativeWindowType CreateWindow()
{
    if ((g_platform = new PlatformData) == 0)
        return 0;

    if(g_platform->Init())
        return (EGLNativeWindowType) g_platform->GetDFBSurface();
    else
        return 0;
}

/*!***********************************************************************
@Function		DestroyWindow
@Input			window		a window handle
@Description	Destroys a window
*************************************************************************/
void DestroyWindow(EGLNativeWindowType window)
{
    if (NULL!=g_platform)
        delete g_platform;

    return;
}
/******************************************************************************
Defines
******************************************************************************/

/*!****************************************************************************
@Function		TestEGLError
@Input			pszLocation		location in the program where the error took
place. ie: function name
@Return		bool			true if no EGL error was detected
@Description	Tests for an EGL error and prints it
******************************************************************************/
bool TestEGLError(const char* pszLocation)
{
    /*
    eglGetError returns the last error that has happened using egl,
    not the status of the last called function. The user has to
    check after every single egl call or at least once every frame.
    */
    EGLint iErr = eglGetError();
    if (iErr != EGL_SUCCESS)
    {
        printf("%s failed (%d).\n", pszLocation, iErr);
        return false;
    }

    return true;
}

/*!****************************************************************************
@Function		main
@Input			argc		Number of arguments
@Input			argv		Command line arguments
@Return		int			result code to OS
@Description	Main function of the program
******************************************************************************/
class TestOGLESHelloWorld : public CTestCase
{
public:

    int main(int argc, char **argv)
    {
        // EGL variables
        EGLDisplay			eglDisplay	= 0;
        EGLConfig			eglConfig	= 0;
        EGLSurface			eglSurface	= 0;
        EGLContext			eglContext	= 0;
        GLuint	ui32Vbo = 0; // Vertex buffer object handle

        EGLNativeWindowType sWindow = CreateWindow();

        if (sWindow == 0)
        {
            printf("[Fatal Error]: Initialize failed --- terminate program !!!");
            return -1;
        }

        /*
        Step 1 - Get the default display.
        EGL uses the concept of a "display" which in most environments
        corresponds to a single physical screen. Since we usually want
        to draw to the main screen or only have a single screen to begin
        with, we let EGL pick the default display.
        Querying other displays is platform specific.
        */
        eglDisplay = eglGetDisplay((NativeDisplayType)0);

        /*
        Step 2 - Initialize EGL.
        EGL has to be initialized with the display obtained in the
        previous step. We cannot use other EGL functions except
        eglGetDisplay and eglGetError before eglInitialize has been
        called.
        If we're not interested in the EGL version number we can just
        pass NULL for the second and third parameters.
        */
        EGLint iMajorVersion, iMinorVersion;
        if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
        {
            printf("Error: eglInitialize() failed.\n");
            goto cleanup;
        }

        /*
        Step 3 - Specify the required configuration attributes.
        An EGL "configuration" describes the pixel format and type of
        surfaces that can be used for drawing.
        For now we just want to use a 16 bit RGB surface that is a
        Window surface, i.e. it will be visible on screen. The list
        has to contain key/value pairs, terminated with EGL_NONE.
        */
        EGLint pi32ConfigAttribs[7];
        pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
        pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
        pi32ConfigAttribs[2] = EGL_RENDERABLE_TYPE;
        pi32ConfigAttribs[3] = EGL_OPENGL_ES_BIT;
        pi32ConfigAttribs[4] = EGL_BUFFER_SIZE;
        pi32ConfigAttribs[5] = 32;
        pi32ConfigAttribs[6] = EGL_NONE;

        /*
        Step 4 - Find a config that matches all requirements.
        eglChooseConfig provides a list of all available configurations
        that meet or exceed the requirements given as the second
        argument. In most cases we just want the first config that meets
        all criteria, so we can limit the number of configs returned to 1.
        */
        EGLint iConfigs;
        if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
        {
            printf("Error: eglChooseConfig() failed.\n");
            goto cleanup;
        }

        /*
        Step 5 - Create a surface to draw to.
        Use the config picked in the previous step and the native window
        handle when available to create a window surface. A window surface
        is one that will be visible on screen inside the native display (or
        fullscreen if there is no windowing system).
        Pixmaps and pbuffers are surfaces which only exist in off-screen
        memory.
        */
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)sWindow, NULL);
        if (!TestEGLError("eglCreateWindowSurface"))
        {
            goto cleanup;
        }

        /*
        Step 6 - Create a context.
        EGL has to create a context for OpenGL ES. Our OpenGL ES resources
        like textures will only be valid inside this context
        (or shared contexts)
        */
        eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
        if (!TestEGLError("eglCreateContext"))
        {
            goto cleanup;
        }

        /*
        Step 7 - Bind the context to the current thread and use our
        window surface for drawing and reading.
        Contexts are bound to a thread. This means you don't have to
        worry about other threads and processes interfering with your
        OpenGL ES application.
        We need to specify a surface that will be the target of all
        subsequent drawing operations, and one that will be the source
        of read operations. They can be the same surface.
        */
        eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
        if (!TestEGLError("eglMakeCurrent"))
        {
            goto cleanup;
        }

        /*
        Step 8 - Draw something with OpenGL ES.
        At this point everything is initialized and we're ready to use
        OpenGL ES to draw something on the screen.
        */

        // Sets the clear color.
        // The colours are passed per channel (red,green,blue,alpha) as float values from 0.0 to 1.0
        glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // clear blue

        // We're going to draw a triangle to the screen so create a vertex buffer object for our triangle
        {
            // Interleaved vertex data
            float afVertices[] = {	-0.4f,-0.4f,0.0f, // Position
                1.0f,1.0f,0.66f,1.0f, // Color
                0.4f,-0.4f,0.0f,
                1.0f,1.0f,0.66f,1.0f,
                0.0f,0.4f,0.0f,
                1.0f,1.0f,0.66f,1.0f};

            // Generate the vertex buffer object (VBO)
            glGenBuffers(1, &ui32Vbo);

            // Bind the VBO so we can fill it with data
            glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

            // Set the buffer's data
            unsigned int uiSize = 3 * (sizeof(float) * 7); // Calc afVertices size (3 vertices * stride (7 verttypes per vertex (3 pos + 4 color)))
            glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);
        }

        // Draws a triangle for 800 frames
        for(int i = 0; i < 800; ++i)
        {
            /*
            Clears the color buffer.
            glClear() can also be used to clear the depth or stencil buffer
            (GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)
            */
            glClear(GL_COLOR_BUFFER_BIT);
            if (!TestEGLError("glClear"))
            {
                goto cleanup;
            }

            /*
            Draw a triangle
            */

            // Enable vertex arrays
            glEnableClientState(GL_VERTEX_ARRAY);

            /*
            Set the vertex pointer.
            param 1: Number of coordinates per vertex; must be 2, 3, or 4.
            param 2: GL_FIXED for CommonLite and GL_FLOAT for Common profile.
            param 3: Specifies the byte offset between consecutive vertexes.
            param 4: Offset to the start of the first vertex into the VBO.
            */
            glVertexPointer(3, GL_FLOAT, sizeof(float) * 7, 0);

            // Set color data in the same way
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4,GL_FLOAT,sizeof(float) * 7, (GLvoid*) (sizeof(float) * 3) /*The color starts after the 3 position values (x,y,z)*/);

            /*
            Draws a non-indexed triangle array from the pointers previously given.
            param 1: Primitive type: GL_POINTS, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, etc.
            param 2: The starting index in the array.
            param 3: The number of indices to be rendered.
            */
            glDrawArrays(GL_TRIANGLES, 0, 3);
            if (!TestEGLError("glDrawArrays"))
            {
                goto cleanup;
            }

            /*
            Swap Buffers.
            Brings to the native display the current render surface.
            */
            eglSwapBuffers(eglDisplay, eglSurface);
            if (!TestEGLError("eglSwapBuffers"))
            {
                goto cleanup;
            }
        }

        /*
        Step 9 - Terminate OpenGL ES and destroy the window (if present).
        eglTerminate takes care of destroying any context or surface created
        with this display, so we don't need to call eglDestroySurface or
        eglDestroyContext here.
        */
cleanup:
        // Delete the VBO as it is no longer needed
        if (ui32Vbo > 0)
            glDeleteBuffers(1, &ui32Vbo);

        DestroyWindow(sWindow);
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
        eglTerminate(eglDisplay);
        return 0;
    }
};

// ADD_TESTCASE(TestOGLESHelloWorld);

/******************************************************************************
End of file (OGLESHelloAPI_NullWS.cpp)
******************************************************************************/

