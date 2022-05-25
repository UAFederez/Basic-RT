#include "Rectangle3D.h"

bool Rectangle3D::hit(const Ray& r, const scalar t_min, const scalar t_max, HitRecord& rec) const
{
    // Calculate the face normal
    Vec3 normal = cross(B - A, D - A);

    // Check if ray intersects the supporting plane
    const scalar denom = -dot(normal, r.direction());

    // Is the ray almost parallel to surface of the plane 
    if(fabs(denom) < 1e-6)
        return false;

    const scalar t = dot(normal, r.origin() - A) / denom;

    if(t_min > t || t > t_max)
        return false;

    // Outside-inside test
    Vec3 Q = r.point_at_t(t);
    scalar c1 = dot(cross((B - A), (Q - A)), normal);
    scalar c2 = dot(cross((C - B), (Q - B)), normal);
    scalar c3 = dot(cross((D - C), (Q - C)), normal);
    scalar c4 = dot(cross((A - D), (Q - D)), normal);

    if(c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0)
        return false;

    if(denom < 0)   // Change this later for non double-sided materials
        normal = -normal;

    //rec.uv = Vec2({ Q.x() + 1, Q.z() + 1 });

    rec.t            = t;
    rec.point_at_t   = r.point_at_t(rec.t);
    rec.normal       = normalize(normal);
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
