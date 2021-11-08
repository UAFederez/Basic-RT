#include "Threading.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

DWORD WINAPI win32_render_tiles(LPVOID param)
{
    thread_render_image_tiles((RenderThreadControl*) param);
    return 0;
}

void initialize_mutex(RenderThreadControl* tcb)
{
    tcb->lock = CreateMutex(NULL,  // Security attributes (default)
                            FALSE, // Initially not owned
                            NULL); // No name
}

int lock_mutex(RenderThreadControl* tcb)
{
    return WaitForSingleObject(tcb->lock, INFINITE);
}

int unlock_mutex(RenderThreadControl* tcb)
{
    return ReleaseMutex(tcb->lock);
}

int create_render_threads(ThreadHandle*  threads, 
                          const uint32_t NUM_THREADS,
                          RenderThreadControl* tcontrol)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        threads[i].handle = CreateThread(NULL,               // Security options (default)
                                         0,                  // Stack size (default)
                                         win32_render_tiles, // thread function
                                         (void*) tcontrol,   // parameter
                                         0,                  // creation flags
                                         NULL);              // return thread ID
        if(threads[i].handle == NULL)
        {
            printf("[ERROR  ] Could not create thread #%d.\n", i);
            return 1;
        }
    }
    return 0;
}

void join_render_threads(ThreadHandle*  threads,
                         const uint32_t NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
        WaitForSingleObject(threads[i].handle, INFINITE);
}


#else

void* pthread_render_tiles(void* param)
{
    thread_render_image_tiles((RenderThreadControl*) param);
    return NULL;
}

void initialize_mutex(RenderThreadControl* tcb)
{
    tcb->lock = PTHREAD_MUTEX_INITIALIZER;
}

int lock_mutex(RenderThreadControl* tcb)
{
    return pthread_mutex_lock(&tcb->lock);
}

int unlock_mutex(RenderThreadControl* tcb)
{
    return pthread_mutex_unlock(&tcb->lock);
}

int create_render_threads(ThreadHandle*  threads, 
                          const uint32_t NUM_THREADS,
                          RenderThreadControl* tcontrol)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_create(&threads[i].handle,
                          NULL,
                          pthread_render_tiles,
                          (void*) tcontrol))
        {
            printf("[ERROR  ] Could not create thread #%d.\n", i);
            return 1;
        }
    }
    return 0;
}

void join_render_threads(ThreadHandle*  threads,
                         const uint32_t NUM_THREADS)
{
    for(uint32_t i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i].handle, NULL);
}

#endif
