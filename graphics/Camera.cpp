#include "Camera.h"
Camera::Camera(const Vec3& lookfrom,
               const Vec3& look_at,
               const Vec3& up,
               const scalar fov,
               const scalar aspect)
{
    scalar theta       = fov * k_PI / 180.0;
    scalar half_height = tan(theta / 2.0);
    scalar half_width  = aspect * half_height;

    origin = lookfrom;
    Vec3 w = normalize(lookfrom - look_at);
    Vec3 u = normalize(cross(up, w));
    Vec3 v = cross(w, u);

    lower_left = Vec3({-half_width, -half_height, -1.0});
    lower_left = origin - half_width * u - half_height * v - w;
    horizontal = 2 * half_width  * u;
    vertical   = 2 * half_height * v;
}
