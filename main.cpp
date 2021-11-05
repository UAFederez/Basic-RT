#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <cfloat>
#include <random>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <sstream>
#include <memory>

#include "math/Vector.h"

#include "util/BitmapImage.h"
#include "util/Threading.h"
#include "util/General.h"

#include "math/Vector3.h"
#include "math/Ray.h"

#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "graphics/Sphere.h"
#include "graphics/Triangle.h"
#include "graphics/Rectangle3D.h"
#include "graphics/Plane.h"
#include "graphics/Scene.h"

Vec3 rotate_y(const Vec3& v, const float theta)
{
    const float rad = theta * M_PI / 180.0f;
    return Vec3({ 
                    dot( v, Vec3({ cos(rad), 0.0, -sin(rad) }) ),
                    dot( v, Vec3({ 0.0, 1.0, 0.0 }) ),
                    dot( v, Vec3({ sin(rad), 0.0, cos(rad) }) ),
               });
                                   
}

Vec3 color(const Ray& r, const Scene& world, int depth)
{
    HitRecord rec = {};
    if(world.anything_hit(r, 1e-3, FLT_MAX, rec))
    {
        Ray  scattered;
        Vec3 attenuation;
        
        if(depth > 50)
           return Vec3({ 0.0, 0.0, 0.0 }); 
        
        Vec3 emitted = rec.material_ptr->emitted();

        if(rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return emitted + attenuation * color(scattered, world, depth + 1);

        return emitted;
    }
    Vec3 unit_dir = normalize(r.direction());
    float t = 0.5 * (unit_dir.y() + 1.0f);
    return (1.0 - t) * Vec3({ 1.0, 1.0, 1.0 }) + t * Vec3({0.5, 0.7, 1.0});
}

int thread_render_image_tiles(RenderThreadControl* tcb)
{
    ImageRenderInfo* image = (ImageRenderInfo*)&tcb->image;

    while(true)
    {
        SectionRenderInfo* current_section = NULL;

        // Check if queue is not empty,
        lock_mutex(tcb);

        if(image->section_queue_front != image->sections.size())
            current_section = &image->sections[image->section_queue_front++];

        unlock_mutex(tcb);

        // color the current section of the image
        if(current_section != NULL)
        {
            const uint32_t bounds_x = current_section->tile_x + current_section->tile_width;
            const uint32_t bounds_y = current_section->tile_y + current_section->tile_height;
            const float NS_DENOM    = 1 / float(image->num_samples);

            for(uint32_t y = current_section->tile_y; y < bounds_y; y++ )
            {
                for(uint32_t x = current_section->tile_x; x < bounds_x; x++ )
                {
                    Vec3 pixel = {};
                    for(uint32_t i = 0; i < image->num_samples; i++)
                    {
                        float u = float(x + random_float()) / float(image->image_width);
                        float v = float(y + random_float()) / float(image->image_height);

                        Ray r  = image->camera->get_ray(u, v);
                        pixel += color(r, *image->world, 0);
                    }
                    pixel *= NS_DENOM;
                    image->pixels[y * image->image_width + x] = pixel; 
                }
            }
            current_section->is_finished = true;
            current_section = NULL;
        } else
            break;

    }
    return 0;
}

// Very basic, not good
void load_mesh_obj_file(const std::string& path, Material* mat, std::vector<Primitive*>* scene)
{
    Vec3 translate = Vec3({ 0.0, 0.0, 2.0 });
    std::ifstream input_file(path);
    std::string line;

    int num_poly = 0;

    int tri_indices[3];
    int verts_read_so_far = 0;

    int nrm_indices[3];
    int norms_read_so_far = 0;

    std::vector<Vec3> vertices       = {};
    std::vector<Vec3> vertex_normals = {};

    std::string dummy_str;
    int  dummy; // To consume unneeded fields for now
    char slash;
    while(std::getline(input_file, line))
    {
        if(line[0] == '#') 
            continue;

        std::istringstream istr(line);

        // Reading a vertex
        if(line.find("v") == 0)
        {
            istr >> slash;
            float x_pos, y_pos, z_pos;
            istr >> x_pos >> y_pos >> z_pos;
            vertices.push_back(0.25 * Vec3({ x_pos, y_pos, z_pos }));
        }

        if(line.find("vn") == 0)
        {
            std::istringstream nrm(line.substr(2));

            float x_nrm, y_nrm, z_nrm;
            nrm >> x_nrm >> y_nrm >> z_nrm;
            vertex_normals.push_back(normalize(Vec3({ x_nrm, y_nrm, z_nrm })));
        }

        // Reading a face
        if(line.find("f") == 0)
        {
            std::string field;
            istr >> field;      // consume 'f'
            while(verts_read_so_far < 3)
            {
                istr >> tri_indices[verts_read_so_far++];

                if(!std::isspace(istr.peek()))
                {
                    istr >> slash;      // consume '/'
                    istr >> dummy;      // vertex texture index
                    istr >> slash;
                    istr >> nrm_indices[norms_read_so_far++];      // vertex normal index
                }
            }

            if(verts_read_so_far == 3)
            {
                Triangle* tri = new Triangle(vertices[tri_indices[0] - 1] + translate,
                                             vertices[tri_indices[1] - 1] + translate,
                                             vertices[tri_indices[2] - 1] + translate,
                                             mat);

                tri->a_nrm = vertex_normals[nrm_indices[0] - 1];
                tri->b_nrm = vertex_normals[nrm_indices[1] - 1];
                tri->c_nrm = vertex_normals[nrm_indices[2] - 1];

                scene->push_back(tri);
                num_poly++;

                verts_read_so_far = 0;
                norms_read_so_far = 0;
            } // TODO: Otherwise it must be an error in the file
        }

        //std::cout << line << '\n';
    }
}

int main()
{
    // Materials
    std::vector<Material*> materials;
    materials.push_back(new Metal(Vec3({0.5, 0.5, 0.5}), 0.10));
    materials.push_back(new Metal(Vec3({0.8, 0.8, 0.8}), 0.10));
    materials.push_back(new Dielectric(1.5));
    materials.push_back(new Lambertian(Vec3({0.9, 0.5, 0.9})));
    materials.push_back(new Emissive(Vec3({10.0, 10.0, 10.0})));
    materials.push_back(new Emissive(Vec3({1.0, 0.7, 0.7})));
    materials.push_back(new Metal(Vec3({0.8, 0.8, 0.9}), 0.05));

    // Scene objects
    Scene scene = {};

    Mesh* teapot_mesh = new Mesh();

    load_mesh_obj_file("meshes/teapot.obj", materials[2], &teapot_mesh->primitives);
    teapot_mesh->calculate_bounding_faces(); 

    scene.meshes.push_back(teapot_mesh);

    //load_mesh_obj_file("meshes/teapot.obj", materials[2], &scene.objects);
    //for(Primitive* p : teapot_mesh->bounding_volume_faces)
    //    scene.objects.push_back(p);

    const float plane_dist = 18.0;
    const float size       = 10.0f;

    const float HALF_SIDE = 1.1f;
    const float height    = 8.0f;

    const auto  transform = [](const Vec3& v, const float theta)
    {
        const Vec3 anchor = Vec3({ 0.0, 0.0, -10 });
        Vec3 translated   = v + anchor;
        Vec3 rotated      = rotate_y(translated, theta);
        Vec3 reverted     = rotated - anchor;

        return reverted;
    };

    const float CEIL_HEIGHT = 50.0f;
    const float CEIL_HALF   = 10.0f;
    const float FLOOR_HALF  = 10.0f;

    // floor
    Mesh* floor_mesh = new Mesh();
    floor_mesh->add_primitive(new Rectangle3D(Vec3({-FLOOR_HALF, -5.0,  plane_dist}),
                                              Vec3({ FLOOR_HALF, -5.0,  FLOOR_HALF * 2.0}),
                                              Vec3({ FLOOR_HALF, -5.0, -FLOOR_HALF * 2.0}),
                                              Vec3({-FLOOR_HALF, -5.0, -FLOOR_HALF * 2.0}),
                                              materials[1]));
    //scene.meshes.push_back(floor_mesh);

    //Mesh* sphere1 = new Mesh();
    //sphere1->add_primitive(new Sphere(Vec3({ 0.0, 0.0, -3.0}), 2.0, materials[0]));
    //scene.meshes.push_back(sphere1);
    //scene.meshes.push_back(sphere2);
    //
    
    for(Mesh* mesh : scene.meshes)
        std::cout << "Checks against " << mesh->bounding_volume_faces.size() << " primitives first\n";

    // Image rendering description
    const uint32_t IMAGE_WIDTH  = 1280;
    const uint32_t IMAGE_HEIGHT = 720;
    const uint32_t TILE_WIDTH   = 16;
    const uint32_t TILE_HEIGHT  = 16;

    // Camera description
    const float view_rot = 90.0f; // 60.0f
    const float dist     = 20.0f; // 8
    Camera main_camera(Vec3({ cos(view_rot * M_PI / 180.0f) * dist, 
                              5.0,
                              sin(view_rot * M_PI / 180.0f) * dist}),
                       Vec3({ 0, 2.5, 6.0}),
                       Vec3({ 0, 1.0, 0}),
                       30.0f, // 30
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // Rendering thread parameters
    const uint32_t MAX_THREADS  = 12; 
    const uint32_t NUM_SAMPLES  = 12;
    const uint32_t NUM_THREADS  = MAX_THREADS;

    std::vector<uint8_t>   image_pixels;

    image_pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    const uint32_t WIDTH_IN_TILES  = IMAGE_WIDTH  / TILE_WIDTH;
    const uint32_t HEIGHT_IN_TILES = IMAGE_HEIGHT / TILE_HEIGHT;
    const uint32_t ADDITIONAL_H    = IMAGE_HEIGHT % TILE_HEIGHT;
    const uint32_t ADDITIONAL_W    = IMAGE_WIDTH % TILE_WIDTH;

    printf("---------------------------------\n");
    printf("Image width:            %d\n", IMAGE_WIDTH);
    printf("Image height:           %d\n", IMAGE_HEIGHT);
    printf("Tile width:             %d\n", TILE_WIDTH);
    printf("Tile height:            %d\n", TILE_HEIGHT);
    printf("Width in tiles:         %d\n", WIDTH_IN_TILES);
    printf("Height in tiles:        %d\n", HEIGHT_IN_TILES);
    printf("Remainder tile width    %d\n", ADDITIONAL_W);
    printf("Remainder tile height:  %d\n", ADDITIONAL_H);
    printf("# of samples per pixel: %d\n", NUM_SAMPLES);
    printf("---------------------------------\n");
    
    RenderThreadControl thread_control;
    thread_control.image = {};
    thread_control.image.image_width  = IMAGE_WIDTH;
    thread_control.image.image_height = IMAGE_HEIGHT;
    thread_control.image.num_samples  = NUM_SAMPLES;
    thread_control.image.world        = &scene;
    thread_control.image.camera       = &main_camera;
    thread_control.image.pixels       = std::vector<Vec3>(IMAGE_WIDTH * IMAGE_HEIGHT);
    thread_control.image.section_queue_front = 0;

    initialize_mutex(&thread_control);

    for(uint32_t tile_y = 0; tile_y < HEIGHT_IN_TILES; tile_y++)
    {
        for(uint32_t tile_x = 0; tile_x < WIDTH_IN_TILES; tile_x++)
        {
            SectionRenderInfo section = {};
            section.tile_width  = TILE_WIDTH;
            section.tile_height = TILE_HEIGHT;
            section.tile_x      = tile_x * TILE_WIDTH;
            section.tile_y      = tile_y * TILE_HEIGHT;
            section.is_finished = false;

            if(tile_x == WIDTH_IN_TILES - 1)
                section.tile_width  += ADDITIONAL_W;
            if(tile_y == HEIGHT_IN_TILES - 1)
                section.tile_height += ADDITIONAL_H;

            thread_control.image.sections.push_back(section);
        }
    }
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::seconds;

    ThreadHandle render_threads[NUM_THREADS];
    
    auto time_render_begin = high_resolution_clock::now();
    std::cout << "[INFO   ] Creating threads...\n";
    if(create_render_threads(render_threads, NUM_THREADS, &thread_control))
    {
        std::cout << "[ERROR  ] Could not create all threads. Exiting now...";
        return -1;
    }
    std::cout << "[SUCCESS] Successfully created all threads\n";

    const uint32_t BAR_WIDTH = 71;
    char  progress_bar[BAR_WIDTH];
    
    progress_bar[BAR_WIDTH - 1] = '\0';
    memset(progress_bar,' ',BAR_WIDTH - 1);
    while(true)
    {
        uint32_t total    = thread_control.image.sections.size();
        uint32_t finished = 0;

        for(const SectionRenderInfo& section : thread_control.image.sections)
        {
            if(section.is_finished)
                finished++;
        }

        const uint32_t num_bars = (float(finished) / float(total)) * (BAR_WIDTH - 1);
        memset(progress_bar,'#', num_bars);

        std::cout << "Rendering image... [" << progress_bar << "]" 
                  << "[" << finished << "/" << total << "]" << '\r';

        if(finished == total)
        {
            std::cout << '\n';
            break;
        }
    }

    join_render_threads(render_threads, NUM_THREADS);
    
    for(Vec3& pixel : thread_control.image.pixels)
    {
        pixel = Vec3({sqrt(pixel.x()), sqrt(pixel.y()), sqrt(pixel.z())});

        int ir = std::min(int(255.99 * pixel.z()), 255);
        int ig = std::min(int(255.99 * pixel.y()), 255);
        int ib = std::min(int(255.99 * pixel.x()), 255);

        image_pixels.push_back(ir);
        image_pixels.push_back(ig);
        image_pixels.push_back(ib);
    }
    auto time_render_end   = high_resolution_clock::now();
    auto time_render_total = duration<double>(time_render_end - time_render_begin).count();

    std::cout << "The render took " << std::fixed << std::setprecision(2)
              << time_render_total  << " seconds.\n";

    write_bmp_to_file("output.bmp", image_pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);

    // Cleanup
    for(Material* mat : materials)
        delete mat;

    return 0;
}
