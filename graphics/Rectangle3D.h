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
        A(v1), B(v2), C(v3), D(v4), material(mat)
    {
    }

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const;
    BoundsDefinition get_bounds() const;

    Vec3 A;
    Vec3 B;
    Vec3 C;
    Vec3 D;
    Material* material = nullptr;
};

#endif
