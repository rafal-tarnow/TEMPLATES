#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/epoll.h>
#include <stdio.h>

Display *dis;
Window win;
int x11_fd;
#define EPOLL_MAX_EVENTS 64
XEvent event;


XConfigureEvent *xc_tmp;
void analyzeX11Events(){

    while(XPending(dis))
    {
        XNextEvent(dis, &event);
        switch (event.type)
        {
        case Expose:
            printf("Expose\n");
            break;
        case ConfigureNotify:
            printf("ConfigureNotify:\n");
            xc_tmp = &(event.xconfigure);
            printf("    x = %d\n", xc_tmp->x);
            printf("    y = %d\n", xc_tmp->y);
            printf("    width = %d\n", xc_tmp->width);
            printf("    height = %d\n", xc_tmp->height);
            break;
        case ButtonPress:
            Button1;
            printf("ButtonPress\n");
            break;
        case ButtonRelease:
            printf("ButtonRelease\n");
            break;
        case MotionNotify:
            printf("MotionNotify\n");
            break;
        case KeyPress:
            printf("KeyPress\n");
            break;
        case KeyRelease:
            printf("KeyRelease\n");
            break;
        case ClientMessage:
            printf("ClientMessage\n");
            //                if (event.xclient.data.l[0] == del_atom)
            //                {
            //                    return 0;
            //                }
            break;
        default:
            printf("Not recognized event type\n");
            break;
        }
    }
}


int main() {
    dis = XOpenDisplay(NULL);
    win = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, 256, 256, 0, BlackPixel (dis, 0), BlackPixel(dis, 0));

    // You don't need all of these. Make the mask as you normally would.
    XSelectInput(dis, win, ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask  | StructureNotifyMask );

    XMapWindow(dis, win);
    XFlush(dis);

    // This returns the FD of the X11 display (or something like that)
    x11_fd = ConnectionNumber(dis);

    //ADD CLIENT TO EPOLL
    int _epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.data.fd = x11_fd;
    event.events = EPOLLIN;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, x11_fd, &event) == -1)
    {
        printf("[ERROR][EPOLL | addClient] Nie można dodać klienta do Epoll'a fd = %d\n", x11_fd);
        return -1;
    }

    //EPOLL MAIN LOOP
    int fileDesc_nums = -1;
    struct epoll_event epoll_events[EPOLL_MAX_EVENTS];

    while(1) {

        fileDesc_nums = epoll_wait(_epoll_fd, epoll_events, EPOLL_MAX_EVENTS, 1000);



        for (int i = 0; i < fileDesc_nums; i++)
        {
            if ((epoll_events[i].events & EPOLLERR) || (epoll_events[i].events & EPOLLHUP) || (!(epoll_events[i].events & EPOLLIN)))
            {
                printf("[ERROR] Blad zdarzenia epoll\n");
                continue;
            }
            if (epoll_events[i].data.fd == x11_fd)
            {
                static int count = 0;
                printf("[INFO] Zdarzenie z X11 %d\n", count++);
                analyzeX11Events();
            }
            else
            {
                printf("[WARNING][EPOLL | runApp] Nie odnaleziono odbiorcy dla zdarzenia fd = %d\n", epoll_events[i].data.fd);
            }
        }


    }

    return(0);
}
