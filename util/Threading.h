#ifndef UTIL_THREADING_H
#define UTIL_THREADING_H

#include <vector>
#include "../graphics/Scene.h"
#include "../graphics/Camera.h"
#include "../math/Vector.h"

struct SectionRenderInfo
{
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t tile_x;
    uint32_t tile_y;
    bool     is_finished;
    bool     in_progress;
};

struct ImageRenderInfo
{
    std::vector<SectionRenderInfo> sections;
    std::vector<Vec3> pixels;

    uint32_t section_queue_front;

    uint32_t total_sections = 0;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t num_samples;
    uint32_t num_finished_tiles = 0;

    Scene*  world;
    Camera* camera;
};

struct RenderThreadControl;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
struct ThreadHandle
{
    HANDLE handle;
};

struct RenderThreadControl
{
    ImageRenderInfo image;
    HANDLE lock;
};

DWORD WINAPI win32_render_tiles(LPVOID param);

#else
#include <pthread.h>
struct ThreadHandle
{
    pthread_t handle;
};


struct RenderThreadControl
{
    ImageRenderInfo image;
    pthread_mutex_t lock;
};

void* pthread_render_tiles(void* param);

#endif

int  thread_render_image_tiles(RenderThreadControl* tcb);
int  lock_mutex               (RenderThreadControl* tcb);
int  unlock_mutex             (RenderThreadControl* tcb);
int  create_render_threads    (ThreadHandle*, const uint32_t, RenderThreadControl*);
void initialize_mutex         (RenderThreadControl*);
void join_render_threads      (ThreadHandle*, const uint32_t);
void cleanup_threads          (RenderThreadControl*, ThreadHandle*, uint32_t);

#endif
