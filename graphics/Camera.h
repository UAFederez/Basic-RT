#ifndef GRAPHICS_CAMERA_H
#define GRAPHICS_CAMERA_H

#include "../math/Vector3.h"
#include "../math/Ray.h"

class Camera {
public:
    Camera(const Vector3& lookfrom,
           const Vector3& look_at,
           const Vector3& up,
           const float    fov,
           const float    aspect)
    {
        float theta       = fov * M_PI / 180.0;
        float half_height = tan(theta / 2.0);
        float half_width  = aspect * half_height;

        origin = lookfrom;
        Vector3 w = normalize(lookfrom - look_at);
        Vector3 u = normalize(cross(up, w));
        Vector3 v = cross(w, u);

        lower_left = Vector3(-half_width, -half_height, -1.0);
        lower_left = origin - half_width * u - half_height * v - w;
        horizontal = 2 * half_width  * u;
        vertical   = 2 * half_height * v;
    }
    Ray get_ray(float s, float t) const
    {
        return Ray(origin, lower_left + s * horizontal + t * vertical - origin);
    }

    Vector3 origin;
    Vector3 lower_left;
    Vector3 horizontal;
    Vector3 vertical;
private:
    
};

#endif
