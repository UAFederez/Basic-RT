#ifndef GEOM_H
#define GEOM_H

#include "Material.h"
#include "../math/Vector.h"
#include "../math/Ray.h"

class Material;

// For constructing axis-aligned bounding boxes
struct BoundsDefinition
{
    Vec3 lower_far_corner;
    Vec3 upper_near_corner;
};

class Primitive {
public:
    Primitive() {}
    virtual bool hit(const Ray&  r, 
                     const float t_min, 
                     const float t_max, 
                     HitRecord&  rec) const = 0;
    virtual ~Primitive() {}
    virtual BoundsDefinition get_bounds() const = 0;
};

#endif
