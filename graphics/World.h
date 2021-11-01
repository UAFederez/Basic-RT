#ifndef GRAPHICS_WORLD_H
#define GRAPHICS_WORLD_H

#include "Geometry.h"

class World {
public:
    bool hit_anything(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
    {
        HitRecord temp_rec = {};
        float closest      = t_max;
        bool hit_anything  = false;

        for(const auto& obj : objects)
        {
            if(obj->hit(r, t_min, closest, temp_rec))
            {
                closest      = temp_rec.t;
                rec          = temp_rec;
                hit_anything = true;
            }
        }
        return hit_anything;
    }
    ~World() {
        for(Geometry* obj : objects)
            delete obj;
    }
    std::vector<Geometry*> objects;
private:
};

#endif
