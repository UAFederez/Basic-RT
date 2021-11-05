#ifndef GRAPHICS_WORLD_H
#define GRAPHICS_WORLD_H

#include "Primitive.h"

struct Light 
{
    Vector3 position;    
};

class Scene {
public:
    bool anything_hit_by_ray(const Ray&  r, 
                             const float t_min, 
                             const float t_max, 
                             HitRecord&  rec) const
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

    // Deallocate scene objects
    ~Scene() 
    {
        for(Primitive* obj : objects)
            delete obj;
    }

    Vector3 light_position;
    std::vector<Primitive*> objects;
private:
};

#endif
