#ifndef UTIL_PROGRESS_BAR_H
#define UTIL_PROGRESS_BAR_H

struct Progress
{
    unsigned width;
    unsigned height;
    unsigned finished;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "ProgressBarWin32.h"

#else
#include "ProgressBarLinux.h"

#endif

void update_progress(Progress* prog)
{
    unsigned total = prog->width * prog->height;
    const int barw = 80;

    char bar[barw];
    memset(bar, ' ', barw);

    float finished_pct = 0;
    while(true)
    {
        finished_pct = float(prog->finished) / float(total);
        int num_bars = finished_pct * barw;
        memset(bar, '#', num_bars);
        printf("Rendering image [");
        printf("%s", bar);
        printf("]");
        printf("[%.2f]", finished_pct * 100.f);

        if(finished_pct == 1.0)
        {
            printf(" DONE!\n");
            break;
        }
        printf("\r");
    }
}


#endif
