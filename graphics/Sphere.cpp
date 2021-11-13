#include "Sphere.h"

bool Sphere::hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
{
    Vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - (4 * a * c);

    if(discriminant > 0)
    {
        float temp = (-b - sqrt(discriminant)) / (2.0 * a); 
        if(t_min < temp && temp < t_max)
        {
            rec.t            = temp;
            rec.point_at_t   = r.point_at_t(temp);
            rec.normal       = (rec.point_at_t - center) / radius;
            rec.material_ptr = material;
            
            // Convert cartesian -> spherical -> UV
            float theta = atan2f(-rec.normal.z(), rec.normal.x()) + k_PI;
            float phi   = 0.5 + asinf(clamp(rec.normal.y(), -1.0f, 1.0f)) / k_PI;
            rec.uv = Vec2({ theta / (2.0f * k_PI), phi });

            rec.tangent   = -normalize(cross(rec.normal, Vec3({ 0.0, 1.0, 0.0 })));
            rec.bitangent =  cross(rec.normal, rec.tangent);

            return true;
        }
        temp = (-b + sqrt(discriminant)) / (2.0 * a);
        if(t_min < temp && temp < t_max)
        {
            rec.t            = temp;
            rec.point_at_t   = r.point_at_t(temp);
            rec.normal       = (rec.point_at_t - center) / radius;
            rec.material_ptr = material;

            // Convert cartesian -> spherical -> UV
            float theta = atan2f(-rec.normal.z(), rec.normal.x()) + k_PI;
            float phi   = 0.5 + asinf(clamp(rec.normal.y(), -1.0f, 1.0f)) / k_PI;
            rec.uv = Vec2({ theta / (2.0f * k_PI), phi });

            rec.tangent   = -normalize(cross(rec.normal, Vec3({ 0.0f, 1.0f, 0.0f })));
            rec.bitangent =  cross(rec.normal, rec.tangent);

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

