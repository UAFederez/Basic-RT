#ifndef GEOM_H
#define GEOM_H

#include "Material.h"
#include "../math/Vector3.h"
#include "../math/Ray.h"

class Material;

class Geometry {
public:
    Geometry() {}
    virtual bool hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const = 0;
    virtual ~Geometry() {}
};

#endif
