#ifndef GRAPHICS_WORLD_H
#define GRAPHICS_WORLD_H

#include "Triangle.h"
#include "Rectangle3D.h"
#include "Sphere.h"
#include "Primitive.h"
#include "Mesh.h"
#include "Plane.h"

#include "../util/BitmapImage.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <cfloat>

struct TextureImage {
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;

    std::string file_path;
    std::vector<Color> colors;       // Floating-point color values used by raytracer
    std::unique_ptr<uint8_t> pixels; // Actual file byte representation

    TextureImage() = default;
};

std::vector<Color> convert_bmp_to_vec3(uint8_t*, const uint32_t, const uint32_t);

class Scene {
public:
    Scene() = default;
    Scene(const std::string& file_path);

    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;

    bool anything_hit(const Ray&  r, 
                      const scalar t_min, 
                      const scalar t_max, 
                      HitRecord&  rec) const;

    // TODO: keep this for now for future performance testing
    bool anything_hit_by_ray(const Ray&  r, 
                             const scalar t_min, 
                             const scalar t_max, 
                             HitRecord&  rec) const;
    
    // TODO: Deallocate any existing objects/values in the scene 
    // object first
    void read_from_file (const std::string& file_path);

    // Deallocate scene objects
    ~Scene();

    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Mesh>>     meshes;

    std::vector<TextureImage > textures;

    std::string name = "output";
    uint32_t image_width;
    uint32_t image_height;
    uint32_t num_samples;
    uint32_t num_threads;

    Vec3 ambient = Vec3({ 0.0, 0.0, 0.0 });
    
    uint32_t num_render_threads;
    
    uint32_t max_recursion_depth = 32;
    uint32_t tile_size = 64;
    Vec3  camera_pos   = Vec3({ 0.0, 0.0,  0.0 });
    Vec3  camera_look  = Vec3({ 0.0, 0.0, -1.0 });
    scalar camera_fov   = 45.0f;
private:
    int  load_texture_image   (const std::string& path);
    void read_scene_parameters(const std::string& line);
    void read_scene_primitives(const std::string& line);
    void read_scene_materials (const std::string& line);
    void load_3d_obj_from_file(const std::string& path, const Vec3& offset, Mesh* mesh, Material* mat);
};

#endif
