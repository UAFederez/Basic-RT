#include "Sphere.h"


bool Sphere::hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
{
    Vec3 oc = r.origin() - center;
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

BoundsDefinition Sphere::get_bounds() const
{
    return BoundsDefinition {
        center - Vec3({ radius, radius, radius }),
        center + Vec3({ radius, radius, radius })
    };
}

