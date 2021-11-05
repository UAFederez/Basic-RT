#ifndef GRAPHICS_PLANE_H
#define GRAPHICS_PLANE_H

#include "Primitive.h"

class Plane : public Primitive {
public:
    Plane(const Vec3& origin, const Vec3& normal, Material* mat):
        normal(normal), origin(origin), material(mat) {}

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        const float denom = -dot(normal, r.direction());
        if(denom > 1e-6)
        {
            const float t = dot(normal, r.origin() - origin) / denom;
            if(t_min < t && t < t_max)
            {
                rec.t            = t;
                rec.point_at_t   = r.point_at_t(rec.t);
                rec.normal       = normal;
                rec.material_ptr = material;
                return true;
            }
            return false;
        }
        return false;
    }
    Vec3 normal;
    Vec3 origin;
    Material* material;
};

#endif
