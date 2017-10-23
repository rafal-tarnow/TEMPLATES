#ifdef __cplusplus
extern “C” {
#endif

#include "glx_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    static GLXFBConfig *fbconfigs;
    static int numfbconfigs;
    static XRenderPictFormat *pict_format;
    static XVisualInfo *visual;
    static Colormap cmap;
    static Window Xroot;
    static Window window_handle;


    static void fatalError(const char *why)
    {
        fprintf(stderr, "%s", why);
        exit(0x666);
    }

    static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
    {
        return d && e && arg && (e->type == MapNotify) && (e->xmap.window == *(Window*)arg);
    }

    static void describe_fbconfig(Display *Xdisplay, GLXFBConfig fbconfig)
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


    void createTheWindow(int *width, int *height, Display* *Xdisplay, int *Xscreen, int * VisData, GLXWindow *glX_window_handle, GLXFBConfig *fbconfig, Atom *del_atom)
    {
        XEvent event;
        //int x,y;
        int attr_mask;
        XSizeHints hints;
        XWMHints *startup_state;
        XTextProperty textprop;
        XSetWindowAttributes attr = {0,};
        static char *title = "FTB's little OpenGL example - ARGB extension by WXD";

        *Xdisplay = XOpenDisplay(NULL);
        if (!*Xdisplay) {
            fatalError("Couldn't connect to X server\n");
        }
        *Xscreen = DefaultScreen(*Xdisplay);
        Xroot = RootWindow(*Xdisplay, *Xscreen);

        fbconfigs = glXChooseFBConfig(*Xdisplay, *Xscreen, VisData, &numfbconfigs);
        *fbconfig = 0;
        for(int i = 0; i<numfbconfigs; i++) {
            visual = (XVisualInfo*) glXGetVisualFromFBConfig(*Xdisplay, fbconfigs[i]);
            if(!visual)
                continue;

            pict_format = XRenderFindVisualFormat(*Xdisplay, visual->visual);
            if(!pict_format)
                continue;

            *fbconfig = fbconfigs[i];
            if(pict_format->direct.alphaMask > 0) {
                break;
            }
        }

        if(!*fbconfig) {
            fatalError("No matching FB config found");
        }

        describe_fbconfig(*Xdisplay, *fbconfig);

        /* Create a colormap - only needed on some X clients, eg. IRIX */
        cmap = XCreateColormap(*Xdisplay, Xroot, visual->visual, AllocNone);

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

        *width = DisplayWidth(*Xdisplay, DefaultScreen(*Xdisplay))/2;
        *height = DisplayHeight(*Xdisplay, DefaultScreen(*Xdisplay))/2;
        int const dim = *width < *height ? *width : *height;

        window_handle = XCreateWindow(	*Xdisplay,
                                        Xroot,
                                        0, 0, dim, dim,
                                        0,
                                        visual->depth,
                                        InputOutput,
                                        visual->visual,
                                        attr_mask, &attr);

        if( !window_handle ) {
            fatalError("Couldn't create the window\n");
        }

#ifdef USE_GLX_CREATE_WINDOW
        int glXattr[] = { None };
        *glX_window_handle = glXCreateWindow(*Xdisplay, *fbconfig, window_handle, glXattr);
        if( !*glX_window_handle ) {
            fatalError("Couldn't create the GLX window\n");
        }
#else
        *glX_window_handle = window_handle;
#endif

        textprop.value = (unsigned char*)title;
        textprop.encoding = XA_STRING;
        textprop.format = 8;
        textprop.nitems = strlen(title);

        hints.width = dim;
        hints.height = dim;
        hints.min_aspect.x = 1;
        hints.min_aspect.y = 1;
        hints.max_aspect.x = 1;
        hints.max_aspect.y = 1;

        hints.flags = USSize|PAspect;

        startup_state = XAllocWMHints();
        startup_state->initial_state = NormalState;
        startup_state->flags = StateHint;

        XSetWMProperties(*Xdisplay, window_handle,&textprop, &textprop,
                         NULL, 0,
                         &hints,
                         startup_state,
                         NULL);

        XFree(startup_state);

        XMapWindow(*Xdisplay, window_handle);
        XIfEvent(*Xdisplay, &event, WaitForMapNotify, (char*)&window_handle);

        if ((*del_atom = XInternAtom(*Xdisplay, "WM_DELETE_WINDOW", 0)) != None) {
            XSetWMProtocols(*Xdisplay, window_handle, &*del_atom, 1);
        }
    }

#ifdef __cplusplus
}
#endif
