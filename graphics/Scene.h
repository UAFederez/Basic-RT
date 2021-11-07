#ifndef GRAPHICS_WORLD_H
#define GRAPHICS_WORLD_H

#include "Triangle.h"
#include "Rectangle3D.h"
#include "Primitive.h"

#include <vector>
#include <cfloat>

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
        if(!bounding_volume_faces.empty())
            bounding_volume_faces.clear();

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
    // Primary intersection function
    bool anything_hit(const Ray&  r, 
                      const float t_min, 
                      const float t_max, 
                      HitRecord&  rec) const;

    // Keep this for now for future testing
    bool anything_hit_by_ray(const Ray&  r, 
                             const float t_min, 
                             const float t_max, 
                             HitRecord&  rec) const;
    // Deallocate scene objects
    ~Scene();

    Vec3 light_position;
    std::vector<Primitive*> objects;
    std::vector<Mesh*> meshes;
private:
};

#endif
