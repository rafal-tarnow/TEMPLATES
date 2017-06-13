#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <stdint.h>

int timer_fd;







int main(int argc, char *argv[])
{ 


//********** TIMER *******************************
    uint64_t missed;
    int ret;
    int loop_count = 0;


    unsigned int period_sec = 0;
    unsigned int period_ns = 1;//0*1000*1000;
    unsigned int start_sec = 5;
    unsigned int start_ns = 0;

    int fd;
    struct itimerspec itval;

    fd = timerfd_create(CLOCK_MONOTONIC, 0/*TFD_NONBLOCK*/);
    timer_fd = fd;
    if (fd == -1)
        return fd;


    itval.it_interval.tv_sec = period_sec;
    itval.it_interval.tv_nsec = period_ns;
    itval.it_value.tv_sec = start_sec;
    itval.it_value.tv_nsec = start_ns;
    ret = timerfd_settime(fd, 0, &itval, NULL);


    while (1) {
        loop_count++;

        ret = read(timer_fd, &missed, sizeof(missed));
        if (ret == -1) {
            perror("read timer error");
            fflush(stdout);
        }else{
            printf("Read = %d Iter = %d\n", (int)missed,loop_count);
            fflush(stdout);
        }
    }

    return 0;
}
