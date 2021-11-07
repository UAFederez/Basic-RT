#include "Scene.h"

bool Scene::anything_hit(const Ray& r, const float t_min, const float t_max, HitRecord& rec) const
{
    HitRecord temp_rec = {};
    float closest      = t_max;
    bool hit_anything  = false;

    HitRecord bv_rec   = {};

    for(const Mesh* mesh : meshes)
    {
        // If mesh hits anything in the bounding_volume_faces
        bool intersects_bv = false;
        for(const Primitive* face : mesh->bounding_volume_faces)
        {
            // Not using FLT_MAX causes farther objects to be 'cut off'
            // because a bounding box face is nearer even though the 
            // area itself may be empty enough to see the further object
            if(face->hit(r, t_min, FLT_MAX, bv_rec))  
                intersects_bv = true;
        }

        // Reset because bounding face is always at least the same distance
        // or closer to the camera than the primitive itself

        /**
         * If so, then check hit with each of the primitives 
         * Hitting a bounding volume does not necessarily mean the ray hits a primitive 
         * if so, this is a false positive, which is fine
         **/
        if(intersects_bv)
        {
            for(const Primitive* prim : mesh->primitives)
            {
                if(prim->hit(r, t_min, closest, temp_rec))
                {
                    closest      = temp_rec.t;
                    rec          = temp_rec;
                    hit_anything = true;
                }
            }
        } // Else, no hit
    }
    return hit_anything;
}

// Keep this for now for future testing
bool Scene::anything_hit_by_ray(const Ray&  r, const float t_min, const float t_max, HitRecord&  rec) const
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
Scene::~Scene() 
{
    for(Primitive* obj : objects)
        delete obj;

    for(Mesh* mesh : meshes)
        delete mesh;
}
