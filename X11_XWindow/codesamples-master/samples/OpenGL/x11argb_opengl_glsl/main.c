/*------------------------------------------------------------------------
 * A demonstration of OpenGL in a  ARGB window
 *    => support for composited window transparency
 *
 * (c) 2011 by Wolfgang 'datenwolf' Draxinger
 *     See me at comp.graphics.api.opengl and StackOverflow.com

 * License agreement: This source code is provided "as is". You
 * can use this source code however you want for your own personal
 * use. If you give this source code to anybody else then you must
 * leave this message in it.
 *
 * This program is based on the simplest possible
 * Linux OpenGL program by FTB (see info below)
 
  The simplest possible Linux OpenGL program? Maybe...

  (c) 2002 by FTB. See me in comp.graphics.api.opengl

  --
  <\___/>
  / O O \
  \_____/  FTB.

------------------------------------------------------------------------*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <sys/types.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <iostream>

using namespace std;

#define USE_CHOOSE_FBCONFIG
//#define USE_GLX_CREATE_WINDOW

#define WINDOW_FULLSCREEN false
#define WINDOW_BORDER true

#define MINIMAL_WINDOW_HEIGHT 240
#define MINIMAL_WINDOW_WIDTH 320



GLuint pbo_id;
GLvoid *pixels;


int v4l2_width = 0;
int v4l2_height = 0;

void GeneratePBO ();

//int x11_fileDescriptor;
fd_set in_fds;
GLubyte *Ytex,*Utex,*Vtex;
FILE *fp;
GLhandleARB FragmentSHandle,ProgramHandle;
GLint i;
char *s;
struct timeval tv;

static const GLchar *vertex_shader_source =
        "#version 120\n"
        "void main()"
        "{"
        "   gl_Position = ftransform();"
        "   gl_TexCoord[0] = gl_MultiTexCoord0;"
        "   gl_FrontColor = gl_Color;"
        "   gl_BackColor = gl_Color;"
        "}\0";
GLuint shaderVertex = 0;



static const GLchar *FragmentCode=
        "uniform sampler2DRect Ytex;\n"
        "uniform sampler2DRect Utex,Vtex;\n"
        "void main(void) {\n"
        "  float nx,ny,r,g,b,y,u,v;\n"
        "  vec4 txl,ux,vx;"
        "  nx=gl_TexCoord[0].x;\n"
        "  ny=720.0-gl_TexCoord[0].y;\n"
        "  y=texture2DRect(Ytex,vec2(nx,ny)).r;\n"
        "  u=texture2DRect(Utex,vec2(nx/2.0,ny/2.0)).r;\n"
        "  v=texture2DRect(Vtex,vec2(nx/2.0,ny/2.0)).r;\n"

        "  y=1.1643*(y-0.0625);\n"
        "  u=u-0.5;\n"
        "  v=v-0.5;\n"

        "  r=y+1.5958*v;\n"
        "  g=y-0.39173*u-0.81290*v;\n"
        "  b=y+2.017*u;\n"

        "  gl_FragColor=vec4(r,g,b,1.0);\n"
        "}\n";

GLuint shaderFragment = 0;

GLuint shaderProgram = 0;





static void fatalError(const char *why)
{
    fprintf(stderr, "%s", why);
    exit(0x666);
}

static int isExtensionSupported(const char *extList, const char *extension)
{

    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if ( where || *extension == '\0' )
        return 0;

    /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
    for ( start = extList; ; ) {
        where = strstr( start, extension );

        if ( !where )
            break;

        terminator = where + strlen( extension );

        if ( where == start || *(where - 1) == ' ' )
            if ( *terminator == ' ' || *terminator == '\0' )
                return 1;

        start = terminator;
    }
    return 0;
}

static int Xscreen;
static Atom del_atom;
static Colormap cmap;
static Display *Xdisplay;
static XVisualInfo *visual;
static XRenderPictFormat *pict_format;
static GLXFBConfig *fbconfigs, fbconfig;
static int numfbconfigs;
static GLXContext render_context;
static Window Xroot, window_handle;
static GLXWindow glX_window_handle;


static int VisData[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, True,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 16,
    None
};

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
    return d && e && arg && (e->type == MapNotify) && (e->xmap.window == *(Window*)arg);
}

static void describe_fbconfig(GLXFBConfig fbconfig)
{
    int doublebuffer;
    int red_bits, green_bits, blue_bits, alpha_bits, depth_bits;

    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_DOUBLEBUFFER, &doublebuffer);
    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_RED_SIZE, &red_bits);
    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_GREEN_SIZE, &green_bits);
    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_BLUE_SIZE, &blue_bits);
    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_ALPHA_SIZE, &alpha_bits);
    glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_DEPTH_SIZE, &depth_bits);

    fprintf(stderr, "FBConfig selected:\n"
                    "Doublebuffer: %s\n"
                    "Red Bits: %d, Green Bits: %d, Blue Bits: %d, Alpha Bits: %d, Depth Bits: %d\n",
            doublebuffer == True ? "Yes" : "No",
            red_bits, green_bits, blue_bits, alpha_bits, depth_bits);
}


void fullscreen(Display* dpy, Window win) {
    Atom atoms[2] = { XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False), None };
    XChangeProperty(
                dpy,
                win,
                XInternAtom(dpy, "_NET_WM_STATE", False),
                XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1
                );
}

static void createTheWindow(bool fullScreen, bool border)
{
    XEvent event;
    int x,y, attr_mask;
    XSizeHints hints;
    XWMHints *startup_state;
    XTextProperty textprop;
    XSetWindowAttributes attr = {0,};
    static const char *title = "FTB's little OpenGL example - ARGB extension by WXD";

    Xdisplay = XOpenDisplay(NULL);
    if (!Xdisplay) {
        fatalError("Couldn't connect to X server\n");
    }
    Xscreen = DefaultScreen(Xdisplay);
    Xroot = RootWindow(Xdisplay, Xscreen);
    // x11_fileDescriptor = ConnectionNumber(Xdisplay);

    fbconfigs = glXChooseFBConfig(Xdisplay, Xscreen, VisData, &numfbconfigs);
    fbconfig = 0;
    for(int i = 0; i<numfbconfigs; i++) {
        visual = (XVisualInfo*) glXGetVisualFromFBConfig(Xdisplay, fbconfigs[i]);
        if(!visual)
            continue;

        pict_format = XRenderFindVisualFormat(Xdisplay, visual->visual);
        if(!pict_format)
            continue;

        fbconfig = fbconfigs[i];
        if(pict_format->direct.alphaMask > 0) {
            break;
        }
    }

    if(!fbconfig) {
        fatalError("No matching FB config found");
    }

    describe_fbconfig(fbconfig);

    /* Create a colormap - only needed on some X clients, eg. IRIX */
    cmap = XCreateColormap(Xdisplay, Xroot, visual->visual, AllocNone);

    attr.colormap = cmap;
    attr.background_pixmap = None;
    attr.border_pixmap = None;
    attr.border_pixel = 0;
    attr.event_mask =
            StructureNotifyMask |
            EnterWindowMask |
            LeaveWindowMask |
            ExposureMask |
            ButtonPressMask |
            ButtonReleaseMask |
            OwnerGrabButtonMask |
            KeyPressMask |
            KeyReleaseMask;

    attr_mask =
            CWBackPixmap|
            CWColormap|
            CWBorderPixel|
            CWEventMask;




    window_handle = XCreateWindow(	Xdisplay, //display
                                        Xroot,	  //window
                                        0, 0,	  //x,y
                                        v4l2_width, v4l2_height, // width, height
                                        10,
                                        visual->depth,
                                        InputOutput,
                                        visual->visual,
                                        attr_mask, &attr);

    if( !window_handle ) {
        fatalError("Couldn't create the window\n");
    }


    if(border == false){
        Atom window_type = XInternAtom(Xdisplay, "_NET_WM_WINDOW_TYPE", False);
        long value = XInternAtom(Xdisplay, "_NET_WM_WINDOW_TYPE_DOCK", False);
        XChangeProperty(Xdisplay, window_handle, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *) &value, 1);
    }

    if(fullScreen == true){
        fullscreen(Xdisplay,window_handle);
    }



#ifdef USE_GLX_CREATE_WINDOW
    int glXattr[] = { None };
    glX_window_handle = glXCreateWindow(Xdisplay, fbconfig, window_handle, glXattr);
    if( !glX_window_handle ) {
        fatalError("Couldn't create the GLX window\n");
    }
#else
    glX_window_handle = window_handle;
#endif

    textprop.value = (unsigned char*)title;
    textprop.encoding = XA_STRING;
    textprop.format = 8;
    textprop.nitems = strlen(title);

    hints.width = v4l2_width;//dim;
    hints.height = v4l2_height;//dim;
    hints.min_width = MINIMAL_WINDOW_WIDTH;
    hints.min_height = MINIMAL_WINDOW_HEIGHT;

    hints.min_aspect.x = 0;
    hints.min_aspect.y = 0;
    hints.max_aspect.x = 100;
    hints.max_aspect.y = 100;

    hints.flags = USSize|PMinSize;

    startup_state = XAllocWMHints();
    startup_state->initial_state = NormalState;
    startup_state->flags = StateHint;

    XSetWMProperties(Xdisplay, window_handle,&textprop, &textprop,
                     NULL, 0,
                     &hints,
                     startup_state,
                     NULL);

    XFree(startup_state);

    XMapWindow(Xdisplay, window_handle);
    XIfEvent(Xdisplay, &event, WaitForMapNotify, (char*)&window_handle);

    if ((del_atom = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", 0)) != None) {
        XSetWMProtocols(Xdisplay, window_handle, &del_atom, 1);
    }
}

static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    fputs("Error at context creation", stderr);
    return 0;
}

static void createTheRenderContext()
{
    int dummy;
    if (!glXQueryExtension(Xdisplay, &dummy, &dummy)) {
        fatalError("OpenGL not supported by X server\n");
    }

#if USE_GLX_CREATE_CONTEXT_ATTRIB
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
    render_context = NULL;
    if( isExtensionSupported( glXQueryExtensionsString(Xdisplay, DefaultScreen(Xdisplay)), "GLX_ARB_create_context" ) ) {
        typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
        if( glXCreateContextAttribsARB ) {
            int context_attribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                None
            };

            int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

            render_context = glXCreateContextAttribsARB( Xdisplay, fbconfig, 0, True, context_attribs );

            XSync( Xdisplay, False );
            XSetErrorHandler( oldHandler );

            fputs("glXCreateContextAttribsARB failed", stderr);
        } else {
            fputs("glXCreateContextAttribsARB could not be retrieved", stderr);
        }
    } else {
        fputs("glXCreateContextAttribsARB not supported", stderr);
    }

    if(!render_context)
    {
#else
    {
#endif
        render_context = glXCreateNewContext(Xdisplay, fbconfig, GLX_RGBA_TYPE, 0, True);
        if (!render_context) {
            fatalError("Failed to create a GL context\n");
        }
    }

    if (!glXMakeContextCurrent(Xdisplay, glX_window_handle, glX_window_handle, render_context)) {
        fatalError("glXMakeCurrent failed for window\n");
    }

    glewInit();
}

static int updateTheMessageQueue()
{
    XEvent event;
    XConfigureEvent *xc;

    while (XPending(Xdisplay))
    {
        XNextEvent(Xdisplay, &event);
        switch (event.type)
        {
        case ClientMessage:
            if (event.xclient.data.l[0] == del_atom)
            {
                return 0;
            }
            break;

        case ConfigureNotify:
            xc = &(event.xconfigure);
            v4l2_width = xc->width;
            v4l2_height = xc->height;
            break;
        }
    }
    return 1;
}

static int check_extensions(void)
{
    if( !GLEW_ARB_vertex_shader ||
            !GLEW_ARB_fragment_shader ) {
        fputs("Required OpenGL functionality not supported by system.\n", stderr);
        return 0;
    }

    return 1;
}



static int init_resources(void)
{


    /////////////////////////////////////////////////////////


    /* Set up program objects. */
    ProgramHandle=glCreateProgramObjectARB();
    FragmentSHandle=glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    /* Compile the shader. */
    glShaderSourceARB(FragmentSHandle,1,&FragmentCode,NULL);
    glCompileShaderARB(FragmentSHandle);

    /* Print the compilation log. */
    glGetObjectParameterivARB(FragmentSHandle,GL_OBJECT_COMPILE_STATUS_ARB,&i);
    s=(char*)malloc(32768);
    glGetInfoLogARB(FragmentSHandle,32768,NULL,s);
    printf("Compile Log: %s\n", s);
    free(s);

    /* Create a complete program object. */
    glAttachObjectARB(ProgramHandle,FragmentSHandle);
    glLinkProgramARB(ProgramHandle);

    /* And print the link log. */
    s=(char*)malloc(32768);
    glGetInfoLogARB(ProgramHandle,32768,NULL,s);
    printf("Link Log: %s\n", s);
    free(s);


    /* Finally, use the program. */
    glUseProgramObjectARB(ProgramHandle);

    int v4l2_width = 0;
    int v4l2_height = 0;

    /* Load the textures. */
    Ytex=(GLubyte *)calloc(v4l2_width*v4l2_height,sizeof(GLubyte));
    Utex=(GLubyte *)calloc((v4l2_width*v4l2_height)/4,sizeof(GLubyte));
    Vtex=(GLubyte *)calloc((v4l2_width*v4l2_height)/4,sizeof(GLubyte));


    memset(Ytex, 128,v4l2_width*v4l2_height);
    memset(Utex, 128,(v4l2_width*v4l2_height)/4);
    memset(Vtex, 128,(v4l2_width*v4l2_height)/4);

    //    size_t size;

    //    fp=fopen("myfile.bin","rb");
    //    size = fread(Ytex,sizeof(GLubyte),230400,fp);

    //    fseek (fp,230400,SEEK_SET);
    //    size = fread(Utex,sizeof(GLubyte),57600,fp);

    //    fseek (fp,230400 + 57600,SEEK_SET);
    //    size = fread(Vtex,sizeof(GLubyte),57600,fp);
    //    fclose(fp);



    /* This might not be required, but should not hurt. */
    glEnable(GL_TEXTURE_2D);

    /* Select texture unit 1 as the active unit and bind the U texture. */
    glActiveTexture(GL_TEXTURE1);
    i=glGetUniformLocationARB(ProgramHandle,"Utex");
    glUniform1iARB(i,1);  /* Bind Utex to texture unit 1 */
    glBindTexture(GL_TEXTURE_RECTANGLE_NV,1);

    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width/2,v4l2_height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,Utex);

    /* Select texture unit 2 as the active unit and bind the V texture. */
    glActiveTexture(GL_TEXTURE2);
    i=glGetUniformLocationARB(ProgramHandle,"Vtex");
    glBindTexture(GL_TEXTURE_RECTANGLE_NV,2);
    glUniform1iARB(i,2);  /* Bind Vtext to texture unit 2 */

    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width/2,v4l2_height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,Vtex);

    /* Select texture unit 0 as the active unit and bind the Y texture. */
    glActiveTexture(GL_TEXTURE0);
    i=glGetUniformLocationARB(ProgramHandle,"Ytex");
    glUniform1iARB(i,0);  /* Bind Ytex to texture unit 0 */
    glBindTexture(GL_TEXTURE_RECTANGLE_NV,3);

    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width,v4l2_height,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,Ytex);

    GeneratePBO();

    return 1;
}



void GeneratePBO ()
{
    pixels = calloc(640*360, sizeof(GLbyte));

    memset (pixels,92,640*360);

    // wygenerowanie identyfikatora obiektu buforowego
    glGenBuffers (1,&pbo_id);

    // wyr�wnywanie wiersza mapy pikselowej do pojedynczego bajta
    glPixelStorei (GL_UNPACK_ALIGNMENT,1);

    // powi�zanie obiektu buforowego
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER,pbo_id);

    // obliczenie rozmiaru bufora na dane
    GLint size = 640*360;


    // za�adowanie danych obiektu buforowego
    glBufferData (GL_PIXEL_UNPACK_BUFFER,size,pixels,GL_STREAM_DRAW);

    // pobranie rozmiaru danych obiektu buforowego
    GLint buf_size;
    glGetBufferParameteriv (GL_PIXEL_UNPACK_BUFFER,GL_BUFFER_SIZE,&buf_size);

    // sprawdzenie poprawno�ci rozmiaru danych obiektu buforowego
    if (buf_size != size)
    {
        printf ("Niepoprawny zapis danych do obiektu buforowego\n");
        exit (0);
    }

    // wy��czenie powi�zania obiektu buforowego
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER,0);


}


static void redrawTheWindow(double T)
{
    int const window_width = v4l2_width;
    int const window_height = v4l2_height;


    glDisable(GL_SCISSOR_TEST);

    glClearColor(0., 0., 0., 1.);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);



    glViewport(0, 0, v4l2_width, v4l2_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,v4l2_width,0,v4l2_height,-10000,10000);



    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -5);

    glDisable(GL_BLEND);



    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    glUseProgramObjectARB(ProgramHandle);




    glBegin(GL_QUADS);
    glTexCoord2i(0,0);            glVertex2i(0,0);
    glTexCoord2i(v4l2_width,0);    	  glVertex2i(v4l2_width,0);
    glTexCoord2i(v4l2_width,v4l2_height); glVertex2i(v4l2_width,v4l2_height);
    glTexCoord2i(0,v4l2_height);            glVertex2i(0,v4l2_height);
    glEnd();






    struct timespec Ta, Tb;

    glXSwapBuffers(Xdisplay, glX_window_handle);
    glXWaitGL();
}


static double getftime(void) {
    static long long offset = 0;
    long long t;
    struct timeval timeofday;

    gettimeofday(&timeofday, NULL);
    t = (long long)timeofday.tv_sec * 1000000 + (long long)timeofday.tv_usec;

    if(offset == 0)
        offset = t;

    return (double)(offset - t) * 1.e-6;
}

#define CLEAR(x) memset(&(x), 0, sizeof(x))


struct buffer {
    void   *start;
    size_t  length;
};


static int fd = -1;
struct buffer          *buffers;
static unsigned int     n_buffers;
static int              force_format;

static void errno_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}






static void process_image(const void *p, int size)
{


    //    fp=fopen("myfile.bin","rb");
    //    fread(Ytex,sizeof(GLubyte),230400,fp);

    //    fseek (fp,230400,SEEK_SET);
    //    fread(Utex,sizeof(GLubyte),57600,fp);

    //    fseek (fp,230400 + 57600,SEEK_SET);
    //    fread(Vtex,sizeof(GLubyte),57600,fp);
    //    fclose(fp);


    char * wsk;
    wsk = (char *)p;

    glActiveTexture(GL_TEXTURE0);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width,v4l2_height,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,wsk);

    glActiveTexture(GL_TEXTURE1);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width/2,v4l2_height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,wsk + v4l2_height*v4l2_width);

    glActiveTexture(GL_TEXTURE2);
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV,0,GL_LUMINANCE,v4l2_width/2,v4l2_height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,wsk + v4l2_height*v4l2_width + (v4l2_height*v4l2_width)/4);



}

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>

int videoFileDescriptor = -1;
struct v4l2_capability * capability;
struct v4l2_format * readedVideoFormat;
struct v4l2_requestbuffers *requestBuffers;
struct v4l2_buffer *memoryMapBuffer;
int bufferSize = 0;
void * buffer;

uint8_t * frame;

void openDevice();
void checkDeviceCapabilities();
void printCapabilities(struct v4l2_capability *);
void resetCropping();
void getVideoFormat();
void printVideoFormat(struct v4l2_format *);
void alocateBuffersForMemoryMapping();
void getBuffersForMemoryMapping();
void streamOn();
void queryBuffer();


int main(int argc, char *argv[])
{

    openDevice();
    checkDeviceCapabilities();
    resetCropping();
    getVideoFormat();

    alocateBuffersForMemoryMapping();
    getBuffersForMemoryMapping();

    //////////////////////////////////////////////////
    static int loop = 0;

    createTheWindow(WINDOW_FULLSCREEN, WINDOW_BORDER);
    createTheRenderContext();
    
    if( !check_extensions() )
        return -1;

    if( !init_resources() )
        return -1;
    //////////////////////////////////////////////////
    /// \brief openDevice
    ///


    streamOn();


    int frameSize = v4l2_height*v4l2_width + (v4l2_height*v4l2_width)/2; //V4L2_PIX_FMT_YUV420 video frame size
    frame = (uint8_t *)calloc(frameSize,sizeof(uint8_t));


    static int frameIndex = 0;
    int returnValue = 0;


    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(videoFileDescriptor, &rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while(1){

        FD_ZERO(&rfds);
        FD_SET(videoFileDescriptor, &rfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        returnValue = select(videoFileDescriptor + 1, &rfds, NULL, NULL, &tv);

        if(returnValue == -1){
            printf("select failed with error %s\n", strerror(errno));
        }else{
            printf("select return value = %d\n",returnValue);
        }

        queryBuffer();
        printf("frameIndex = %d\n", frameIndex++);

        process_image(buffer,frameSize);
        redrawTheWindow(getftime());
    }


    while(1){


        // for(int i =0; i < frameSize; i++){
        //     frameCopy[i] = frame[i];
        // }

        // printf("Read frame return value %d\n",returnValue);

        /*if(returnValue == -1){

        }else if(returnValue == 0){
            //printf();
        }else{
            //printf();
        }*/

        // printf("Frame index = %d\n", frameIndex++);

        //  process_image(frame,frameSize);
        //  redrawTheWindow(getftime());
    }

    return 0;
}

void openDevice(){
    videoFileDescriptor = open("/dev/video0",O_RDWR & (~O_NONBLOCK));
    if(videoFileDescriptor == -1){
        printf("Device failed to open with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        printf("Device was open successful with fileDescriptor = %d\n", videoFileDescriptor);
    }
}

void queryBuffer(){
    struct v4l2_buffer buffer;

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.index = 0;
    buffer.flags = 0;
    buffer.reserved = 0;
    buffer.reserved2 = 0;
    buffer.memory = V4L2_MEMORY_MMAP;

    int ret;
    ret = ioctl(videoFileDescriptor,VIDIOC_QBUF,&buffer);







    if(ret == -1){
        printf("VIDIOC_QBUF ioctl failure with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(ret == 0){
        printf("VIDIOC_QBUF ioctl OK\n");
    }else{
        printf("VIDIOC_QBUF ioctl return unknown\n");
    }

    ret = ioctl(videoFileDescriptor,VIDIOC_DQBUF,&buffer);

}

void streamOn(){
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = ioctl(videoFileDescriptor,VIDIOC_STREAMON,&type);

    if(ret == -1){
        printf("VIDIOC_STREAMON ioctl failure with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(ret == 0){
        printf("VIDIOC_STREAMON ioctl OK\n");
    }else{
        printf("VIDIOC_STREAMON ioctl return unknown\n");
    }
}

void getBuffersForMemoryMapping(){
    memoryMapBuffer = (struct v4l2_buffer *)calloc(1,sizeof(struct v4l2_buffer));

    memoryMapBuffer->index = 0;
    memoryMapBuffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    memoryMapBuffer->reserved = 0;
    memoryMapBuffer->reserved2 = 0;

    int ret = ioctl(videoFileDescriptor,VIDIOC_QUERYBUF,memoryMapBuffer);


    if(ret == -1){
        printf("VIDIOC_QUERYBUF ioctl failure with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(ret == 0){
        printf("VIDIOC_QUERYBUF ioctl OK\n");
        printf("   v4l2_buffer->flags: %d\n",memoryMapBuffer->flags);
        switch (memoryMapBuffer->memory) {
        case V4L2_MEMORY_MMAP:
            printf("    v4l2_buffer->memory: V4L2_MEMORY_MMAP\n");
            break;
        case V4L2_MEMORY_USERPTR:
            printf("    v4l2_buffer->memory: V4L2_MEMORY_USERPTR\n");
            break;
        case V4L2_MEMORY_OVERLAY:
            printf("    v4l2_buffer->memory: V4L2_MEMORY_OVERLAY\n");
            break;
        case V4L2_MEMORY_DMABUF:
            printf("   v4l2_buffer->memory: V4L2_MEMORY_DMABUF\n");
            break;
        default:
            printf("   v4l2_buffer->memory: Unrecognized\n");
            break;
        }
        printf("   v4l2_buffer->lenght: %d\n",memoryMapBuffer->length);
    }else{
        printf("VIDIOC_QUERYBUF ioctl unknow return code\n");
        exit(EXIT_FAILURE);
    }

    bufferSize = memoryMapBuffer->length;
    buffer = mmap(NULL, memoryMapBuffer->length, PROT_READ | PROT_WRITE, /* recommended */ MAP_SHARED, /* recommended */ videoFileDescriptor, memoryMapBuffer->m.offset);

    if (MAP_FAILED == buffer) {
        /* If you do not exit here you should unmap() and free()
               the buffers mapped so far. */
        printf("Buffers mmap failed\n");
        exit(EXIT_FAILURE);
    }
}

void alocateBuffersForMemoryMapping(){
    requestBuffers = (struct v4l2_requestbuffers *)calloc(1,sizeof(struct v4l2_requestbuffers));

    requestBuffers->count = 1;
    requestBuffers->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffers->memory = V4L2_MEMORY_MMAP;

    int ret = ioctl(videoFileDescriptor,VIDIOC_REQBUFS,requestBuffers);

    if(ret == 0){
        printf("VIDIOC_REQBUFS ioctl return %d allocated buffers\n", requestBuffers->count);
        if(requestBuffers->count == 0){
            exit(EXIT_FAILURE);
        }
    }else if(ret == -1){
        printf("VIDIOC_REQBUFS ioctl failure with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        printf("VIDIOC_REQBUFS ioctl unknow return code\n");
        exit(EXIT_FAILURE);
    }
}


void checkDeviceCapabilities(){
    capability = (struct v4l2_capability *)calloc(1,sizeof(struct v4l2_capability));

    int ret = ioctl(videoFileDescriptor,VIDIOC_QUERYCAP,capability);
    if(ret == 0){
        printf("VIDIOC_QUERYCAP ioctl OK\n");
        printCapabilities(capability);
    }else if(ret == -1){
        printf("VIDIOC_QUERYCAP ioctl failure with error %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else{
        printf("VIDIOC_QUERYCAP ioctl unknow return code\n");
        exit(EXIT_FAILURE);
    }
}

void printCapabilities(struct v4l2_capability * cap){
    printf("   Capability driver: %s\n",cap->driver);
    printf("   Capability card: %s\n",cap->card);
    printf("   Capability bus info: %s\n",cap->bus_info);
    printf("   Capability version: %u.%u.%u\n", (cap->version >> 16) & 0xFF, (cap->version >> 8) & 0xFF, cap->version & 0xFF);
    printf("   Capability capabilities: %x\n",cap->capabilities);
    printf("     The device supports the single-planar API through the Video Memory-To-Memory interface [");(cap->capabilities & V4L2_CAP_VIDEO_M2M)?printf("YES"):printf("NO");printf("]\n");
    printf("     The device supports the multi-planar API through the Video Memory-To-Memory interface [");(cap->capabilities & V4L2_CAP_VIDEO_M2M_MPLANE)?printf("YES"):printf("NO");printf("]\n");
    printf("     The device supports the Video Overlay interface [");(cap->capabilities & V4L2_CAP_VIDEO_OVERLAY)?printf("YES"):printf("NO");printf("]\n");
    printf("     The device supports the read() and/or write() I/O methods [");(cap->capabilities & V4L2_CAP_READWRITE)?printf("YES"):printf("NO");printf("]\n");
    printf("     The device supports the asynchronous I/O methods [");(cap->capabilities & V4L2_CAP_ASYNCIO)?printf("YES"):printf("NO");printf("]\n");
    printf("     The device supports the streaming I/O method (memory mapping) [");(cap->capabilities & V4L2_CAP_STREAMING)?printf("YES"):printf("NO");printf("]\n");
    printf("   Capability device_caps: %x\n",cap->device_caps);
}

struct v4l2_cropcap cropcap;
struct v4l2_crop crop;

void resetCropping(){
    memset (&cropcap, 0, sizeof (cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int ret = ioctl (videoFileDescriptor, VIDIOC_CROPCAP, &cropcap);

    if (ret == -1){
        printf ("VIDIOC_CROPCAP iocts cropping capabilities failed with error %s\n", strerror(errno));
        //exit (EXIT_FAILURE);
    }else if(ret == 0){
        printf ("VIDIOC_CROPCAP iocts cropping capabilities read OK");
    }else{
        printf ("VIDIOC_CROPCAP iocts cropping capabilities unknow return code");
        exit (EXIT_FAILURE);
    }

    memset (&crop, 0, sizeof (crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;

    /* Ignore if cropping is not supported (EINVAL). */

    ret = ioctl (videoFileDescriptor, VIDIOC_S_CROP, &crop);
    if (ret == -1) {
        printf ("VIDIOC_S_CROP iocts set cropping failed with error %s\n", strerror(errno));
        //exit (EXIT_FAILURE);
    }else if(ret == 0){
        printf ("VIDIOC_S_CROP iocts set cropping OK");
    }else{
        printf ("VIDIOC_S_CROP iocts set cropping unknow return code");
        exit (EXIT_FAILURE);
    }
}

void getVideoFormat(){
    readedVideoFormat = (struct v4l2_format *)calloc(1,sizeof(struct v4l2_format));
    readedVideoFormat->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = ioctl (videoFileDescriptor, VIDIOC_G_FMT, readedVideoFormat);
    if(ret == -1){
        printf ("VIDIOC_G_FMT iocts get video format failed with error %s\n", strerror(errno));
        exit (EXIT_FAILURE);
    }else if(ret == 0){
        printf ("VIDIOC_G_FMT iocts get video format OK\n");
        printVideoFormat(readedVideoFormat);
    }else{
        printf ("VIDIOC_G_FMT iocts get video format unknow result\n");
        exit (EXIT_FAILURE);
    }
}

void printVideoFormat(struct v4l2_format * format){
    printf("   Video format width: %d\n",format->fmt.pix.width);
    printf("   Video format height: %d\n",format->fmt.pix.height);
    v4l2_height = format->fmt.pix.height;
    v4l2_width = format->fmt.pix.width;
    printf("   Video format pixelformat: %d\n",format->fmt.pix.pixelformat);

    switch (format->fmt.pix.pixelformat) {
    case V4L2_PIX_FMT_RGB332:
        printf("   Video format pixelformat: V4L2_PIX_FMT_RGB332\n");
        break;
    case V4L2_PIX_FMT_YVU420:
        printf("   Video format pixelformat: V4L2_PIX_FMT_YVU420\n");
        break;
    case V4L2_PIX_FMT_YUV420:
        printf("   Video format pixelformat: V4L2_PIX_FMT_YUV420\n");
        break;
    default:
        printf("   Video format pixelformat: Not recognized\n");
        break;
    }

    switch (format->fmt.pix.field) {
    case V4L2_FIELD_ANY:
        printf("   Video format field: V4L2_FIELD_ANY\n");
        break;
    case V4L2_FIELD_NONE:
        printf("   Video format field: V4L2_FIELD_NONE\n");
        break;
    case V4L2_FIELD_TOP:
        printf("   Video format field: V4L2_FIELD_TOP\n");
        break;
    case V4L2_FIELD_BOTTOM:
        printf("   Video format field: V4L2_FIELD_BOTTOM\n");
        break;
    case V4L2_FIELD_INTERLACED:
        printf("   Video format field: V4L2_FIELD_INTERLACED\n");
        break;
    case V4L2_FIELD_SEQ_TB:
        printf("   Video format field: V4L2_FIELD_SEQ_TB\n");
        break;
    case V4L2_FIELD_SEQ_BT:
        printf("   Video format field: V4L2_FIELD_SEQ_BT\n");
        break;
    case V4L2_FIELD_ALTERNATE:
        printf("   Video format field: V4L2_FIELD_ALTERNATE\n");
        break;
    case V4L2_FIELD_INTERLACED_TB:
        printf("   Video format field: V4L2_FIELD_INTERLACED_TB\n");
        break;
    case V4L2_FIELD_INTERLACED_BT:
        printf("   Video format field: V4L2_FIELD_INTERLACED_BT\n");
        break;
    default:
        printf("   Video format field: Not recognized\n");
        break;
    }
}








