#ifndef RAYTRACE_RAY_H
#define RAYTRACE_RAY_H

#include "Vector3.h"

class Ray {
public:
    Ray() { }
    Ray(const Vector3& orig, const Vector3& dir): orig(orig), dir(dir) { }

    Vector3 origin()    const { return orig; }
    Vector3 direction() const { return dir;  }
    Vector3 point_at_t(const float t) const { return orig + (t * dir); }
    Vector3 orig;
    Vector3 dir;
};

#endif
