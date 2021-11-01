#ifndef GRAPHICS_TRIANGLE_H
#define GRAPHICS_TRIANGLE_H

#include "Geometry.h"

class Triangle : public Geometry {
public:
    Triangle(const Vector3& v0, 
             const Vector3& v1,
             const Vector3& v2,
             Material* material):
        v0(v0), v1(v1), v2(v2), material(material),
        normal(cross(v1 - v0, v2 - v0))
    {
    }
    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {

        const float denom = -dot(normal, r.direction());
        if(denom < 1e-6)
            return false;

        const float t = dot(normal, r.origin() - v0) / denom;
        if(t_min > t || t > t_max)
            return false;

        Vector3 p = r.point_at_t(t);

        Vector3 e0 = v1 - v0;
        Vector3 e1 = v2 - v1;
        Vector3 e2 = v0 - v2;

        Vector3 c0 = p - v0;
        Vector3 c1 = p - v1;
        Vector3 c2 = p - v2;
        
        if(dot(normal, cross(e0, c0)) > 0 &&
           dot(normal, cross(e1, c1)) > 0 &&
           dot(normal, cross(e2, c2)) > 0)
        {
            rec.t            = t;
            rec.point_at_t   = r.point_at_t(rec.t);
            rec.normal       = normal;
            rec.material_ptr = material;
            return true;
        }
        return false;
    }
    
    Vector3 v0;
    Vector3 v1;
    Vector3 v2;
    Material* material;
    Vector3 normal;
private:
};

#endif
