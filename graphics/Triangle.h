#ifndef GRAPHICS_TRIANGLE_H
#define GRAPHICS_TRIANGLE_H

#include "Primitive.h"

/**
 * Vertices must be specified in counterclockwise order
 **/
class Triangle : public Primitive {
public:
    Triangle(const Vec3& v0, 
             const Vec3& v1,
             const Vec3& v2,
             Material* material):
        A(v0), 
        B(v1), 
        C(v2), 
        material(material)
    {
    }
    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        // Face normal
        Vec3 normal = normalize(cross(B - A, C - A));

        // Check if the point is within the supporting plane
        const float denom = -dot(normal, r.direction());

        // TODO: maybe add option for single vs. double-sided
        // fabs to allow viewing from both sides
        //
        if(denom < 1e-3 && !material->is_double_sided)
            return false;

        const float t = dot(normal, r.origin() - A) / denom;

        if(t_min > t || t > t_max)
            return false;

        // Outside-inside test
        Vec3  Q = r.point_at_t(t);
        float CAB  = dot(cross((B - A), (Q - A)), normal);
        float CCB  = dot(cross((C - B), (Q - B)), normal);
        float CAC  = dot(cross((A - C), (Q - C)), normal);

        if(CAB < 0 || CCB < 0 || CAC < 0)
            return false;

        float area = dot(cross((B - A), (C - A)), normal);

        float alpha = CCB / area;
        float beta  = CAC / area;
        float gamma = CAB / area;

        // If vertex normals have been explicitly defined
        if(a_nrm.magnitude_squared() != 0 &&
           b_nrm.magnitude_squared() != 0 &&
           c_nrm.magnitude_squared() != 0 )
            normal = normalize((a_nrm * alpha) + (b_nrm * beta) + (c_nrm * gamma));

        if(denom < 0 && material->is_double_sided)
            normal = -normal;

        rec.uv           = Vec2({ beta, gamma });
        rec.t            = t;
        rec.point_at_t   = r.point_at_t(rec.t);
        rec.normal       = normal;
        rec.material_ptr = material;
        return true;
    }

    BoundsDefinition get_bounds() const
    {
        Vec3 low_far = A;
        Vec3 up_near = A;

        Vec3 vertices[3] = { A, B, C };

        for(const Vec3 v : vertices)
        {
            for(int i = 0; i < 3; i++)
                low_far[i] = std::min(low_far[i], v[i]);

            for(int i = 0; i < 3; i++)
                up_near[i] = std::max(up_near[i], v[i]);
        }

        return BoundsDefinition { low_far, up_near };
    }

    // Vertex normals
    Vec3 a_nrm;
    Vec3 b_nrm;
    Vec3 c_nrm;
    
    Vec3 A;
    Vec3 B;
    Vec3 C;
    Material* material = nullptr;
private:
};

#endif
