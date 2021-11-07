#ifndef GRAPHICS_CAMERA_H
#define GRAPHICS_CAMERA_H

#include "../math/Vector.h"
#include "../math/Ray.h"

class Camera {
public:
    Camera(const Vec3& lookfrom,
           const Vec3& look_at,
           const Vec3& up,
           const float fov,
           const float aspect);

    inline Ray get_ray(float s, float t) const
    {
        return Ray(origin, lower_left + s * horizontal + t * vertical - origin);
    }

    Vec3 origin;
    Vec3 lower_left;
    Vec3 horizontal;
    Vec3 vertical;
private:
    
};

#endif
