#ifndef GRAPHICS_TRIANGLE_H
#define GRAPHICS_TRIANGLE_H

#include "Geometry.h"

/**
 * Vertices must be specified in counterclockwise order
 **/
class Triangle : public Geometry {
public:
    Triangle(const Vector3& v0, 
             const Vector3& v1,
             const Vector3& v2,
             Material* material):
        A(v0), 
        B(v1), 
        C(v2), 
        material(material)
    {
    }
    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        Vector3 normal = normalize(cross(B - A, C - A));

        // Check if the point is within the supporting plane
        const float denom = -dot(normal, r.direction());
        if(denom < 1e-6)
            return false;

        const float t = dot(normal, r.origin() - A) / denom;

        if(t_min > t || t > t_max)
            return false;

        // Outside-inside test
        Vector3 Q = r.point_at_t(t);
        float CAB = dot(cross((B - A), (Q - A)), normal);
        float CCB = dot(cross((C - B), (Q - B)), normal);
        float CAC = dot(cross((A - C), (Q - C)), normal);

        if(CAB < 0 || CCB < 0 || CAC < 0)
            return false;
        
        rec.t            = t;
        rec.point_at_t   = r.point_at_t(rec.t);
        rec.normal       = normal;
        rec.material_ptr = material;
        return true;
    }
    
    Vector3 A;
    Vector3 B;
    Vector3 C;
    Material* material;
private:
};

#endif
