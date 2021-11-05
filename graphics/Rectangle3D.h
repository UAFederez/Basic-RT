#ifndef GRAPHICS_RECTANGLE_H
#define GRAPHICS_RECTANGLE_H

#include "Primitive.h"

class Rectangle3D : public Primitive {
public:
    Rectangle3D(const Vec3& v1,
                const Vec3& v2,
                const Vec3& v3,
                const Vec3& v4,
          Material* mat):
        A(v1), B(v2), C(v3), D(v4),
        material(mat)
    {
    }

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        // Calculate the face normal
        Vec3 normal = normalize(cross(B - A, D - A));

        // Check if ray intersects the supporting plane
        const float denom = -dot(normal, r.direction());

        // Is the ray almost parallel to surface of the plane 
        if(denom < 1e-6)
            return false;

        const float t = dot(normal, r.origin() - A) / denom;

        if(t_min > t || t > t_max)
            return false;

        // Outside-inside test
        Vec3 Q = r.point_at_t(t);
        float c1 = dot(cross((B - A), (Q - A)), normal);
        float c2 = dot(cross((C - B), (Q - B)), normal);
        float c3 = dot(cross((D - C), (Q - C)), normal);
        float c4 = dot(cross((A - D), (Q - D)), normal);

        if(c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0)
            return false;

        rec.t            = t;
        rec.point_at_t   = r.point_at_t(rec.t);
        rec.normal       = normal;
        rec.material_ptr = material;
        return true;
    }
    Vec3 A;
    Vec3 B;
    Vec3 C;
    Vec3 D;
    Material* material;
};

#endif
