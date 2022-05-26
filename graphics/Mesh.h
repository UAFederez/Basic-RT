#ifndef GRAPHICS_MESH_H
#define GRAPHICS_MESH_H

#include "Triangle.h"
#include "Rectangle3D.h"
#include "Primitive.h"

#include <vector>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>

struct Mesh
{
    Mesh();

    /**
     * Adds a primitive to the collection of primitives to render
     * Memory ownership must be transferred to the Mesh object
     **/
    void add_primitive(Primitive* p);

    /**
     * Determines the axis-aligned bounding box, composed of 6
     * Rectangle3D's which encloses all of the primitives of this
     * mesh
     **/
    void calculate_bounding_faces();

    ~Mesh();

    std::vector<Material* > materials;
    std::vector<Primitive*> primitives;
    std::vector<Primitive*> bounding_volume_faces;
    Material* bv_mat;
};

#endif
