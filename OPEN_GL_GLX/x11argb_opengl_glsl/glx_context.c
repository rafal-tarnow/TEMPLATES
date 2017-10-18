#ifdef __cplusplus
extern “C” {
#endif

#include "glx_context.h"
#include <stdio.h>


    static void fatalError(const char *why)
    {
        fprintf(stderr, "%s", why);
        exit(0x666);
    }

    static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
    {
        fputs("Error at context creation", stderr);
        return 0;
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


     void createTheRenderContext(Display * *Xdisplay,  GLXWindow *glX_window_handle, GLXFBConfig *fbconfig, GLXContext *render_context)
    {
        int dummy;
        if (!glXQueryExtension(*Xdisplay, &dummy, &dummy)) {
            fatalError("OpenGL not supported by X server\n");
        }

#if USE_GLX_CREATE_CONTEXT_ATTRIB
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
        *render_context = NULL;
        if( isExtensionSupported( glXQueryExtensionsString(*Xdisplay, DefaultScreen(*Xdisplay)), "GLX_ARB_create_context" ) ) {
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

                *render_context = glXCreateContextAttribsARB( *Xdisplay, *fbconfig, 0, True, context_attribs );

                XSync( *Xdisplay, False );
                XSetErrorHandler( oldHandler );

                fputs("glXCreateContextAttribsARB failed", stderr);
            } else {
                fputs("glXCreateContextAttribsARB could not be retrieved", stderr);
            }
        } else {
            fputs("glXCreateContextAttribsARB not supported", stderr);
        }

        if(!*render_context)
        {
#else
        {
#endif
            *render_context = glXCreateNewContext(*Xdisplay, *fbconfig, GLX_RGBA_TYPE, 0, True);
            if (!*render_context) {
                fatalError("Failed to create a GL context\n");
            }
        }

        if (!glXMakeContextCurrent(*Xdisplay, *glX_window_handle, *glX_window_handle, *render_context)) {
            fatalError("glXMakeCurrent failed for window\n");
        }


    }








#ifdef __cplusplus
}
#endif
