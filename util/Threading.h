#ifndef UTIL_THREADING_H
#define UTIL_THREADING_H

#include "../graphics/World.h"
#include "../graphics/Camera.h"

#include "../math/Vector3.h"

struct SceneInfo
{
    Camera* camera;
    World*  world;
};

struct ThreadImageInfo
{
    uint32_t thread_id;
    uint32_t num_samples;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t finished_pixels;
    std::vector<Vector3> pixels;

    SceneInfo  scene;
};

// TODO: may be parameterized but left as is for now
void thread_render_image(ThreadImageInfo* job);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>

struct ThreadHandle
{
    HANDLE handle;    
};

DWORD win32_render_image(LPVOID job_ptr)
{
    thread_render_image((ThreadImageInfo*) job_ptr);
    return 0;
}

int create_render_threads(ThreadImageInfo* jobs, 
                           ThreadHandle*   thread_handles,
                           const uint32_t  NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        thread_handles[i].handle = CreateThread(NULL,               // Security attributes (default)
                                                0,                  // Default stack size (default)
                                                win32_render_image, // thread function
                                                (void*) &jobs[i],   // Argument to thread function
                                                0,                  // Creation flags (default)
                                                NULL);
        if(thread_handles[i].handle == NULL)
        {
            printf("[ERROR] Could not create Win32 thread #%d\n", i);
            return -1;
        }
    }
    return 0;
}

void join_threads(ThreadHandle* thread_handles, const uint32_t NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
        WaitForSingleObject(thread_handles[i].handle, INFINITE);
}

#else
#include <pthread.h>

struct ThreadHandle
{
    pthread_t handle;
};


void* pthread_render_image(void* job_ptr)
{
    thread_render_image((ThreadImageInfo*) job_ptr);
    return NULL;
}

int create_render_threads(ThreadImageInfo* jobs, 
                           ThreadHandle*   thread_handles,
                           const uint32_t  NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_create(&thread_handles[i].handle, // pthread handle
                          NULL,                      // thread attributes
                          pthread_render_image,      // thread function
                          (void*) &jobs[i]))         // thread parameter
        {
            printf("[ERROR] Could not create pthread #%d\n", i);
            return -1;
        }
    }
    return 0;
}

void join_threads(ThreadHandle* thread_handles, const uint32_t NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
        pthread_join(thread_handles[i].handle, NULL);
}
#endif

#endif
