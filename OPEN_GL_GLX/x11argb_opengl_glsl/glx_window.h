#pragma once

#ifdef __cplusplus
extern “C” {
#endif

#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>

#define USE_GLX_CREATE_WINDOW

    void createTheWindow(int *width, int *height, Display* *Xdisplay, int *Xscreen, int * VisData, GLXWindow *glX_window_handle, GLXFBConfig *fbconfig, Atom *del_atom);


#ifdef __cplusplus
}
#endif
