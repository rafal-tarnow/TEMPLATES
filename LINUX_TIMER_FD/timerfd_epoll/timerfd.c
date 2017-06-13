#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <sys/epoll.h>

int timer_fd;



int main(int argc, char *argv[])
{ 


    //********** TIMER *******************************
    uint64_t missed;
    int ret;
    int loop_count = 0;


    unsigned int period_sec = 1;
    unsigned int period_ns = 0;//0*1000*1000;
    unsigned int start_sec = 5;
    unsigned int start_ns = 0;

    int timer_fd;
    struct itimerspec itval;

    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1)
        return timer_fd;


    itval.it_interval.tv_sec = period_sec;
    itval.it_interval.tv_nsec = period_ns;
    itval.it_value.tv_sec = start_sec;
    itval.it_value.tv_nsec = start_ns;
    ret = timerfd_settime(timer_fd, 0, &itval, NULL);

    //*********** EPOLL *********************************

#define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    int epoll_fd, nfds;


    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    //ev.events = EPOLLET;  EDGE TRIGERRING
    ev.events = EPOLLIN;
    ev.data.fd = timer_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }



    //******* LOOP ***************************************



    for (;;) {
        loop_count++;
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        printf("epoll_wait = %d\n", nfds);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        int n = 0;
        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == timer_fd) {
                printf("Loop timer fd loop = %d\n ",loop_count);
                ret = read(timer_fd, &missed, sizeof(missed));
                if (ret == -1) {
                    perror("read timer error");
                    fflush(stdout);
                }else{
                    printf("Read = %d Iter = %d\n", (int)missed,loop_count);
                    fflush(stdout);
                }
            }
        }
    }

    return 0;
}
