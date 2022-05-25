#include "Triangle.h"

bool Triangle::hit(const Ray& r, 
                   const scalar t_min, 
                   const scalar t_max, 
                   HitRecord& rec) const
{
    // Face normal
    Vec3 normal = cross(B - A, C - A);

    // Check if the point is within the supporting plane
    const scalar denom = -dot(normal, r.direction());

    if(denom < 1e-6 && !material->is_double_sided)
        return false;

    const scalar t = dot(normal, r.origin() - A) / denom;

    if(t_min > t || t > t_max)
        return false;

    // Outside-inside test
    const Vec3  Q    = r.point_at_t(t);
    const scalar CAB = dot(cross((B - A), (Q - A)), normal);
    const scalar CCB = dot(cross((C - B), (Q - B)), normal);
    const scalar CAC = dot(cross((A - C), (Q - C)), normal);

    if(CAB < 0 || CCB < 0 || CAC < 0)
        return false;

    // Changed from cross(B - A, C - A) because it is the same
    const scalar area  = dot(normal, normal);
    const scalar darea = 1.0 / area;

    const scalar alpha = CCB * darea;
    const scalar beta  = CAC * darea;
    const scalar gamma = (1 - alpha - beta);

    // If vertex normals have been explicitly defined
    if(a_nrm.magnitude_squared() != 0 &&
       b_nrm.magnitude_squared() != 0 &&
       c_nrm.magnitude_squared() != 0 )
        normal = (a_nrm * alpha) + (b_nrm * beta) + (c_nrm * gamma);

    if(denom < 0 && material->is_double_sided)
        normal = -normal;

    rec.uv           = Vec2({ beta, gamma });
    rec.t            = t;
    rec.point_at_t   = r.point_at_t(rec.t);
    rec.normal       = normalize(normal);
    rec.material_ptr = material;
    return true;
}

BoundsDefinition Triangle::get_bounds() const
{
    Vec3 low_far = A;
    Vec3 up_near = A;

    Vec3 vertices[3] = { A, B, C };

    for(const Vec3 v : vertices)
    {
        for(int i = 0; i < 3; i++)
        {
            low_far[i] = std::min(low_far[i], v[i]);
            up_near[i] = std::max(up_near[i], v[i]);
        }
    }

    return BoundsDefinition { low_far, up_near };
}


