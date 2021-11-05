#ifndef GRAPHICS_SPHERE_H
#define GRAPHICS_SPHERE_H

#include "Primitive.h"

class Sphere : public Primitive {
public:
    Sphere() {}

    Sphere(const Vector3& center, const float radius, Material* material):
        center(center), radius(radius), material(material) {}

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        Vector3 oc = r.origin() - center;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - (a * c);

        if(discriminant > 0)
        {
            float temp = (-b - sqrt(discriminant)) / (a); 
            if(t_min < temp && temp < t_max)
            {
                rec.t            = temp;
                rec.point_at_t   = r.point_at_t(temp);
                rec.normal       = (rec.point_at_t - center) / radius;
                rec.material_ptr = material;
                return true;
            }
            temp = (-b + sqrt(discriminant)) / (a);
            if(t_min < temp && temp < t_max)
            {
                rec.t            = temp;
                rec.point_at_t   = r.point_at_t(temp);
                rec.normal       = (rec.point_at_t - center) / radius;
                rec.material_ptr = material;
                return true;
            }
        }
        return false;
    }

    ~Sphere() {
    }
private:
    Vector3   center;
    float     radius;
    Material* material;
};

#endif
