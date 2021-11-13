#ifndef GRAPHICS_SPHERE_H
#define GRAPHICS_SPHERE_H

#include "Primitive.h"

class Sphere : public Primitive {
public:
    Sphere() {}

    Sphere(const Vec3& center, 
           const float radius, 
           Material* material):
        center(center), 
        radius(radius), 
        material(material) 
    {
    }

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const;
    BoundsDefinition get_bounds() const;
private:
    Vec3      center = Vec3();
    float     radius = 0.0f;
    Material* material = nullptr;
};

#endif
