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
            
            // Convert cartesian -> spherical -> UV
            double theta = atan2(-rec.normal.z(), rec.normal.x()) + k_PI;
            double phi   = 0.5 + asin(rec.normal.y()) / k_PI;
            rec.uv = Vec2({ theta / (2.0 * k_PI), phi });
            
            rec.tangent   = -normalize(cross(rec.normal, Vec3({ 0.0, 1.0, 0.0 })));
            rec.bitangent =  normalize(cross(rec.normal, rec.tangent));

            return true;
        }
        temp = (-b + sqrt(discriminant)) / (a);
        if(t_min < temp && temp < t_max)
        {
            rec.t            = temp;
            rec.point_at_t   = r.point_at_t(temp);
            rec.normal       = (rec.point_at_t - center) / radius;
            rec.material_ptr = material;

            // Convert cartesian -> spherical -> UV
            double theta = atan2(-rec.normal.z(), rec.normal.x()) + k_PI;
            double phi   = 0.5 + asin(rec.normal.y()) / k_PI;
            rec.uv = Vec2({ theta / (2.0 * k_PI), phi });

            rec.tangent   = -normalize(cross(rec.normal, Vec3({ 0.0, 1.0, 0.0 })));
            rec.bitangent =  normalize(cross(rec.normal, rec.tangent));

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

