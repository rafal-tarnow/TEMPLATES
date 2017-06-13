/*#include <sys/timerfd.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    int timer_fd;
    timer_fd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK);
    if(timer_fd == -1){
        printf("Failed cerate timer fd\n");
    }else if(timer_fd != -1){
        printf("Sucessfull created timer fd\n");
    }


    struct itimerspec new_value;

    new_value.it_interval.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;
    new_value.it_value.tv_sec = 1;


    int settime_ret = timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &new_value, NULL);
    if(settime_ret == 0){
        printf("Sucessfull set time\n");
    }else if(settime_ret == -1){
        printf("Failed set time \n");
    }
    fflush(stdout);


    while (1) {

    }

    return 0;
}*/


#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void
print_elapsed_time(void)
{
    static struct timespec start;
    struct timespec curr;
    static int first_call = 1;
    int secs, nsecs;

    if (first_call) {
        first_call = 0;
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
            handle_error("clock_gettime");
    }

    if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
        handle_error("clock_gettime");

    secs = curr.tv_sec - start.tv_sec;
    nsecs = curr.tv_nsec - start.tv_nsec;
    if (nsecs < 0) {
        secs--;
        nsecs += 1000000000;
    }
    printf("%d.%03d: ", secs, (nsecs + 500000) / 1000000);
}

int
main(int argc, char *argv[])
{
    int start_delay = 0;
    int counts = 10000;
    unsigned int period_ms = 1;

    struct itimerspec new_value;
    int fd;
    struct timespec now;
    uint64_t exp, tot_exp;
    ssize_t s;


    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
        handle_error("clock_gettime");


    new_value.it_value.tv_sec = now.tv_sec + start_delay;
    new_value.it_value.tv_nsec = now.tv_nsec;

    unsigned int ns = period_ms*1000000;


    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = ns;


    fd = timerfd_create(CLOCK_REALTIME, 0);

    timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL);

    print_elapsed_time();
    printf("timer started\n");

    for (tot_exp = 0; tot_exp < counts;) {
        s = read(fd, &exp, sizeof(uint64_t));
        if (s != sizeof(uint64_t))
            handle_error("read");

        tot_exp += exp;
        print_elapsed_time();
        printf("read: %llu; total=%llu\n",
               (unsigned long long) exp,
               (unsigned long long) tot_exp);
    }

    exit(EXIT_SUCCESS);
}

