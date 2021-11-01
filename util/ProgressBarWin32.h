#ifndef UTIL_PROGRESS_BAR_WIN32_H
#define UTIL_PROGRESS_BAR_WIN32_H

#include <windows.h>

void update_progress(Progress*);

struct ThreadInfo
{
    HANDLE thread_handle; 
};

DWORD WINAPI update_progress_w32(LPVOID progress)
{
    update_progress((Progress*) progress);
    return 0;
}

int create_thread(Progress* prog, ThreadInfo* info)
{
    info->thread_handle = CreateThread(NULL,                // Security attributes
                                      0,                    // Default stack size
                                      update_progress_w32,  // Thread function
                                      (void*) prog,         // thread parameter
                                      0,                    // Creation flags
                                      NULL);                // Return thread id
    if(info->thread_handle == NULL)
        printf("[ERROR] Could not create thread!\n");

    return 0;
}

void join_thread(ThreadInfo* info)
{
    WaitForSingleObject(info->thread_handle, INFINITE);
}

#endif
