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
        BA(normalize(B-A)),
        CA(normalize(C-A)),
        material(material)
    { }

    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const;
    BoundsDefinition get_bounds() const;

    // Vertex normals
    Vec3 a_nrm;
    Vec3 b_nrm;
    Vec3 c_nrm;
    
    Vec3 A;
    Vec3 B;
    Vec3 C;
    Vec3 BA;
    Vec3 CA;
    Material* material = nullptr;
private:
};

#endif
