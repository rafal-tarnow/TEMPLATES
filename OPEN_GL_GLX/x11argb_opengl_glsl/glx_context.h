#pragma once

#ifdef __cplusplus
extern “C” {
#endif

#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>


  void createTheRenderContext(Display * *Xdisplay,  GLXWindow *glX_window_handle, GLXFBConfig *fbconfig, GLXContext *render_context);

#ifdef __cplusplus
}
#endif
