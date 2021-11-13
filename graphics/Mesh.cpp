#include "Mesh.h"

Mesh::Mesh()
{
    bv_mat = new Lambertian(Vec3({ 1.0, 0.0, 1.0 }));
}

void Mesh::add_primitive(Primitive* p)
{
    primitives.push_back(p);
    calculate_bounding_faces();
}

// Note: any vertex transformation may invalidate the bounding box
void Mesh::calculate_bounding_faces()
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
        {
            low_far[i] = std::min(low_far[i], defn.lower_far_corner[i]);
            up_near[i] = std::max(up_near[i], defn.upper_near_corner[i]);
        }
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

Mesh::~Mesh()
{
    delete bv_mat;

    for(Primitive* p : bounding_volume_faces)
        delete p;

    for(Primitive* p : primitives)
        delete p;
}
