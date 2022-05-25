#include "Plane.h"
#include <cfloat>

bool Plane::hit(const Ray& r, 
                const scalar t_min, 
                const scalar t_max, 
                HitRecord& rec) const
{
    const scalar denom = -dot(normal, r.direction());
    if(denom > 1e-6)
    {
        const scalar t = dot(normal, r.origin() - origin) / denom;
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

BoundsDefinition Plane::get_bounds() const
{
    return BoundsDefinition {
        Vec3({ -FLT_MAX, -FLT_MAX, -FLT_MAX }),
        Vec3({  FLT_MAX,  FLT_MAX,  FLT_MAX }),
    };
}

