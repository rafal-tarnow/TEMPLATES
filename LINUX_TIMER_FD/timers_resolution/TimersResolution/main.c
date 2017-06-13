#include <stdio.h>
#include <time.h>

int main(void)
{
    clockid_t clocks[] = {
        CLOCK_REALTIME,
        CLOCK_MONOTONIC,
        CLOCK_PROCESS_CPUTIME_ID,
        CLOCK_THREAD_CPUTIME_ID,
        CLOCK_MONOTONIC_RAW,
        (clockid_t) -1
    };
    int i;

    for(i = 0; clocks[i] != (clockid_t) -1; i++)
    {
        struct timespec res;
        int ret;
        ret = clock_getres(clocks[i],&res);
        if(ret)
            perror("clock_getres");
        else
            printf("zegar=%d sekundy=%1d nanosekundy=%1d\n", clocks[i], res.tv_sec, res.tv_nsec);
    }

    printf("Hello World!\n");
    return 0;
}

