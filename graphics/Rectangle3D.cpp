#include "Rectangle3D.h"

bool Rectangle3D::hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
{
    // Calculate the face normal
    Vec3 normal = normalize(cross(B - A, D - A));

    // Check if ray intersects the supporting plane
    const float denom = -dot(normal, r.direction());

    // Is the ray almost parallel to surface of the plane 
    if(fabs(denom) < 1e-6)
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

    if(denom < 0)   // Change this later for non double-sided materials
        normal = -normal;

    rec.t            = t;
    rec.point_at_t   = r.point_at_t(rec.t);
    rec.normal       = normal;
    rec.material_ptr = material;
    return true;
}


BoundsDefinition Rectangle3D::get_bounds() const
{
    Vec3 low_far = A;
    Vec3 up_near = A;

    Vec3 vertices[4] = { A, B, C, D };

    for(const Vec3 v : vertices)
    {
        if(v.x() < low_far.x())
            low_far[0] = v.x();
        if(v.y() < low_far.y())
            low_far[1] = v.y();
        if(v.z() < low_far.z())
            low_far[2] = v.z();

        if(v.x() > up_near.x())
            up_near[0] = v.x();
        if(v.y() > up_near.y())
            up_near[1] = v.y();
        if(v.z() > up_near.z())
            up_near[2] = v.z();
    }

    return BoundsDefinition { low_far, up_near };
}
