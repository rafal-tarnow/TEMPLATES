#pragma once

#include <GL/glew.h>

#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>
#include <GL/glx.h>


int drawing_init_resources(void);
void drawing_redrawTheWindow(Display * *Xdisplay, GLXWindow * glX_window_handle, int *width, int *height, double T);
