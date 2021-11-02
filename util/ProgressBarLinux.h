#ifndef UTIL_PROGRESS_BAR_LINUX_H
#define UTIL_PROGRESS_BAR_LINUX_H

#include <pthread.h>

void update_progress(Progress*);

struct ThreadInfo
{
    pthread_t thread_handle;
};

void* update_progress_linux(void* progress)
{
    update_progress((Progress*) progress);
    return NULL;
}

int create_thread(Progress* prog, ThreadInfo* info)
{
    if(pthread_create(&info->thread_handle, NULL, update_progress_linux, (void*) prog))
        printf("[ERROR] Could not create thread!\n");
    return 0;
}

void join_thread(ThreadInfo* info)
{
    pthread_join(info->thread_handle, NULL);
}

#endif
