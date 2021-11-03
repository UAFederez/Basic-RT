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

#endif
