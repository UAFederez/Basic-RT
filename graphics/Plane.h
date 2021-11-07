#ifndef GRAPHICS_PLANE_H
#define GRAPHICS_PLANE_H

#include "Primitive.h"

class Plane : public Primitive {
public:
    Plane(const Vec3& origin, 
          const Vec3& normal, 
          Material* mat):
        normal(normal), origin(origin), material(mat) 
    {
    }

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const;
    BoundsDefinition get_bounds() const;

    Vec3 normal;
    Vec3 origin;
    Material* material = nullptr;
};

#endif
