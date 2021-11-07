#ifndef GRAPHICS_WORLD_H
#define GRAPHICS_WORLD_H

#include "Triangle.h"
#include "Rectangle3D.h"
#include "Primitive.h"

struct Mesh
{
    Mesh()
    {
        bv_mat = new Lambertian(Vec3({ 1.0, 0.0, 1.0 }));
    }

    void add_primitive(Primitive* p)
    {
        primitives.push_back(p);
        calculate_bounding_faces();
    }

    // Note: any vertex transformation may invalidate the bounding box
    void calculate_bounding_faces()
    {
        // Step 1 : Determine near and far corners
        //
        // Since the only primitives are triangles do this for now,
        // Include the bounds get() methods on the base primitive class later
        Vec3 low_far = { Vec3({  FLT_MAX, FLT_MAX, FLT_MAX }) };
        Vec3 up_near = { Vec3({ -FLT_MAX,-FLT_MAX,-FLT_MAX }) };

        for(const Primitive* p : primitives)
        {
            BoundsDefinition defn = p->get_bounds();

            for(std::size_t i = 0; i < 3; i++)
                low_far[i] = std::min(low_far[i], defn.lower_far_corner[i]);

            for(std::size_t i = 0; i < 3; i++)
                up_near[i] = std::max(up_near[i], defn.upper_near_corner[i]);
        }

        // Step 2 : Construct rectangular prism based on corners
        bounding_volume_faces.push_back(new Rectangle3D(Vec3({ low_far.x(), low_far.y(), up_near.z() }),
                                                        Vec3({ up_near.x(), low_far.y(), up_near.z() }),
                                                        up_near,
                                                        Vec3({ low_far.x(), up_near.y(), up_near.z() }),
                                                        bv_mat));
        bounding_volume_faces.push_back(new Rectangle3D(low_far,
                                                        Vec3({ low_far.x(), low_far.y(), up_near.z() }),
                                                        Vec3({ low_far.x(), up_near.y(), up_near.z() }),
                                                        Vec3({ low_far.x(), up_near.y(), low_far.z() }),
                                                        bv_mat));
        bounding_volume_faces.push_back(new Rectangle3D(Vec3({ up_near.x(), low_far.y(), up_near.z() }),
                                                        Vec3({ up_near.x(), up_near.y(), up_near.z() }),
                                                        Vec3({ up_near.x(), up_near.y(), low_far.z() }),
                                                        Vec3({ up_near.x(), low_far.y(), low_far.z() }),
                                                        bv_mat));
        bounding_volume_faces.push_back(new Rectangle3D(low_far,
                                                        Vec3({ up_near.x(), low_far.y(), low_far.z() }),
                                                        Vec3({ up_near.x(), up_near.y(), low_far.z() }),
                                                        Vec3({ low_far.x(), up_near.y(), low_far.z() }),
                                                        bv_mat));
        bounding_volume_faces.push_back(new Rectangle3D(Vec3({ low_far.x(), up_near.y(), up_near.z() }),
                                                        up_near,
                                                        Vec3({ up_near.x(), up_near.y(), low_far.z() }),
                                                        Vec3({ low_far.x(), up_near.y(), low_far.z() }),
                                                        bv_mat));
        bounding_volume_faces.push_back(new Rectangle3D(Vec3({ low_far.x(), low_far.y(), up_near.z() }),
                                                        Vec3({ up_near.x(), low_far.y(), up_near.z() }),
                                                        Vec3({ up_near.x(), low_far.y(), low_far.z() }),
                                                        low_far,
                                                        bv_mat));
    }

    std::vector<Material*  > materials;
    std::vector<Primitive* > primitives;
    std::vector<Primitive* > bounding_volume_faces;
    Material* bv_mat;

    ~Mesh()
    {
        delete bv_mat;
        for(Primitive* p : bounding_volume_faces)
            delete p;
    }
};

struct Light 
{
    Vec3 position;    
};

class Scene {
public:
    bool anything_hit(const Ray&  r,
                      const float t_min,
                      const float t_max,
                      HitRecord&  rec) const
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

        for(Mesh* mesh : meshes)
            delete mesh;
    }

    Vec3 light_position;
    std::vector<Primitive*> objects;

    std::vector<Mesh*> meshes;
private:
};

#endif
