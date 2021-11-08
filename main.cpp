#include <SFML/Graphics.hpp>

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
    const float rad = theta * k_PI / 90.0f;
    return Vec3({ 
                    dot( v, Vec3({ cos(rad), 0.0, -sin(rad) }) ),
                    dot( v, Vec3({ 0.0, 1.0, 0.0 }) ),
                    dot( v, Vec3({ sin(rad), 0.0, cos(rad) }) ),
               });
                                   
}

Color color(const Ray& r, const Scene& world, int depth)
{
    HitRecord rec = {};
    if(world.anything_hit(r, 1e-3, FLT_MAX, rec))
    {
        Ray   scattered;
        Color attenuation;
        
        if(depth > 50)
           return Color({ 0.0, 0.0, 0.0 }); 
        
        Vec3 emitted = rec.material_ptr->emitted();

        if(rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return emitted + attenuation * color(scattered, world, depth + 1);

        return emitted;
    }
    return Vec3({ 0.01, 0.01, 0.035 });
    //Vec3 unit_dir = normalize(r.direction());
    //double t = 0.5 * (unit_dir.y() + 1.0);
    //return (1.0 - t) * Color({ 1.0, 1.0, 1.00 }) + t * Color({0.7, 0.7, 1.0});
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
            const double   NS_DENOM = 1 / double(image->num_samples);
            const double   IW_DENOM = 1 / double(image->image_width);
            const double   IH_DENOM = 1 / double(image->image_height);

            current_section->in_progress = true;
            for(uint32_t y = current_section->tile_y; y < bounds_y; y++ )
            {
                for(uint32_t x = current_section->tile_x; x < bounds_x; x++ )
                {
                    Vec3 pixel = {};
                    for(uint32_t i = 0; i < image->num_samples; i++)
                    {
                        double u = double(x + random_float()) * IW_DENOM;
                        double v = double(y + random_float()) * IH_DENOM;

                        Ray r  = image->camera->get_ray(u, v);
                        pixel += color(r, *image->world, 0);
                    }
                    pixel *= NS_DENOM;
                    image->pixels[y * image->image_width + x] = pixel; 
                }
            }
            current_section->is_finished = true;
            current_section->in_progress = false;
            current_section = NULL;
        } else
            break;

    }
    return 0;
}

// Very basic, not good
void load_mesh_obj_file(const std::string& path, 
                        Material* mat, 
                        std::vector<Primitive*>* scene,
                        const Vec3 move   = Vec3({ 0.0, 0.0, 0.0}))
{
    Vec3 translate = Vec3({ 0.0, 0.0, 0.0 }) + move;
    std::ifstream input_file(path);
    std::string line;

    if(!input_file)
    {
        std::printf("[ERROR] Could not read \'%s\'. Stopping file input\n", path.c_str());
        return;
    }

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

                    if(istr.peek() == '/')
                        istr >> slash;      // consume '/'
                    else
                    {
                        istr >> dummy;      // vertex texture index
                        istr >> slash;
                    }
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
    std::cout << path << " has " << num_poly << " faces.\n";
    //std::cout << "DONE READING...\n";
}

int main()
{
    // Materials
    std::vector<Material*> materials;
    materials.push_back(new Metal(Color({0.9, 0.9, 0.9}), 0.5));
    materials.push_back(new Lambertian(Color({0.9, 0.9, 0.9})));
    materials.push_back(new Dielectric(1.5));

    materials.push_back(new Lambertian(Color({0.5, 0.5, 0.5})));
    materials.push_back(new Emissive(Color({2.5, 2.25, 2.0})));

    materials.push_back(new Emissive(Color({2.0, 2.0, 2.0})));
    materials.push_back(new Emissive(Color({2.0, 2.0, 2.0})));
    materials.push_back(new Lambertian(Color({0.7, 0.2, 0.7})));
    materials.push_back(new Lambertian(Color({0.23, 0.37, 0.7})));
    materials.push_back(new Metal(Color({0.8, 0.8, 0.9}), 0.05));

    // Scene objects
    Scene scene = {};

    Mesh* model_mesh = new Mesh();
    model_mesh->add_primitive(new Sphere(Vec3({ 0.0, 1.0, 0.0 }), 1.0, materials[2]));
    scene.meshes.push_back(model_mesh);

    /**
    Mesh* model_mesh2 = new Mesh();
    load_mesh_obj_file("meshes/monkey.obj", 
                        materials[4], 
                        &model_mesh2->primitives,
                        Vec3({ -0.5, 0.5, 0.0 }));
    model_mesh2->calculate_bounding_faces(); 
    scene.meshes.push_back(model_mesh2);

    Mesh* model_mesh3 = new Mesh();
    load_mesh_obj_file("meshes/monkey.obj", 
                        materials[1], 
                        &model_mesh3->primitives,
                        Vec3({  0.5, 0.5, 0.0 }));
    model_mesh3->calculate_bounding_faces(); 
    scene.meshes.push_back(model_mesh3);
    **/


    //Mesh* model_mesh2 = new Mesh();

    //load_mesh_obj_file("meshes/superman-low-poly-smooth.obj", 
    //                    materials[2], 
    //                    &model_mesh2->primitives,
    //                    Vec3({ 0, 0.0, 0.0 }));
    //model_mesh2->calculate_bounding_faces(); 

    //scene.meshes.push_back(model_mesh2);

    const float plane_dist = 18.0;
    const float FLOOR_HALF = 10.0f;

    Mesh* lighting1 = new Mesh();
    lighting1->name = "Lighting 1";
    lighting1->add_primitive(new Sphere(Vec3({ 2.1, 1.0, 0.0}), 1.0, materials[0]));
    scene.meshes.push_back(lighting1);

    Mesh* lighting2 = new Mesh();
    lighting2->name = "Lighting 2";
    lighting2->add_primitive(new Sphere(Vec3({-2.1, 1, 0.0}), 1.0, materials[1]));
    scene.meshes.push_back(lighting2);

    Mesh* lighting3 = new Mesh();
    lighting3->name = "Lighting 2";
    lighting3->add_primitive(new Sphere(Vec3({ 0.0, 5, -5.0}), 1.0, materials[4]));
    scene.meshes.push_back(lighting3);

    // floor
    const float size       = 10.0f;

    const float HALF_SIDE = 0.6f;
    const float height    = 2.0f;
    const float CEIL_HEIGHT = 50.0f;
    const float CEIL_HALF   = 10.0f;

    Mesh* floor_mesh = new Mesh();
    floor_mesh->name = "Floor mesh";
    floor_mesh->add_primitive(new Rectangle3D(Vec3({-FLOOR_HALF, -0.0,  plane_dist}),
                                              Vec3({ FLOOR_HALF, -0.0,  FLOOR_HALF * 2.0}),
                                              Vec3({ FLOOR_HALF, -0.0, -FLOOR_HALF * 2.0}),
                                              Vec3({-FLOOR_HALF, -0.0, -FLOOR_HALF * 2.0}),
                                              materials[8]));
    scene.meshes.push_back(floor_mesh);

    const auto  transform = [](const Vec3& v, const float theta) 
    {
        const Vec3 anchor = Vec3({ 0.0, 0.0, (v.z() <= 0.0 ? -3.0 : 3.0) });
        Vec3 translated   = v + anchor;
        Vec3 rotated      = rotate_y(translated, theta);
        Vec3 reverted     = rotated - anchor;

        return reverted;
    };
    float theta = -30;
    
    for(int i = 0; i < 5; i++)
    {
        Mesh* lighting_rect = new Mesh();

        const float ldist = 5.0;
        const float y_up  = 0.25;

        const float t = (float(i) / float(5));

        /**
        Material* mat = new Metal(Vec3({ (1.0 - t) * (0.75 + random_float(0, 0.25)),
                                         0.25 + random_float(0, 0.1),
                                         t * 0.75 + random_float(0, 0.25) }), 0.0 );
                                            
        lighting_rect->add_primitive(new Triangle(transform(Vec3({ -HALF_SIDE, y_up + 0.0, -ldist }), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + 0.0, -ldist}), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + height, -ldist }), theta * 2),
                                                  mat));

        lighting_rect->add_primitive(new Triangle(transform(Vec3({ -HALF_SIDE, y_up + 0.0, -ldist }), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + height, -ldist }), theta * 2),
                                                  transform(Vec3({ -HALF_SIDE, y_up + height, -ldist }), theta * 2),
                                                  mat));
        materials.push_back(mat);

        // Front lights
        Material* mat2 = new Emissive(Vec3({ 0.75 + random_float(0, 0.25),
                                             0.75 + random_float(0, 0.25),
                                             0.75 + random_float(0, 0.25) }) );
                                            

        lighting_rect->add_primitive(new Triangle(transform(Vec3({ -HALF_SIDE, y_up + 0.0, 5.0 }), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + 0.0, 5.0}), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + height, 5.0 }), theta * 2),
                                                  mat2));

        lighting_rect->add_primitive(new Triangle(transform(Vec3({ -HALF_SIDE, y_up + 0.0, 5.0 }), theta * 2),
                                                  transform(Vec3({  HALF_SIDE, y_up + height, 5.0 }), theta * 2),
                                                  transform(Vec3({ -HALF_SIDE, y_up + height, 5.0 }), theta * 2),
                                                  mat2));
        materials.push_back(mat2);
        theta += 15;

        scene.meshes.push_back(lighting_rect);
        **/
    }

    //scene.meshes.push_back(sphere2);
    //
    //
    //scene.objects.push_back(new Sphere(Vec3({ 0.0, 0.0, -3.0}), 2.0, materials[0]));
    
    for(Mesh* mesh : scene.meshes)
        std::cout << mesh->name << " require intersection check against " 
                                 << mesh->bounding_volume_faces.size() << " primitives first\n";

    // Image rendering description
    const uint32_t IMAGE_WIDTH  = 1280;
    const uint32_t IMAGE_HEIGHT = 720;
    const uint32_t TILE_WIDTH   = 64;
    const uint32_t TILE_HEIGHT  = 64;

    // Camera description
    const float view_rot = 90.0f; // 60.0f
    const float dist     =  5.0; // 8
    Camera main_camera(Vec3({ cos(view_rot * k_PI / 180.0f) * dist, 
                              3.0,
                              sin(view_rot * k_PI / 180.0f) * dist}),
                       Vec3({ 0, 1.0, 0.0 }),
                       Vec3({ 0, 1.0, 0}),
                       45.0f, // 30
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // Rendering thread parameters
    const uint32_t MAX_THREADS  = 12;
    const uint32_t NUM_SAMPLES  = 1000;
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
            section.in_progress = false;

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

    // GUI Display
    sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Render");
    sf::Image output;
    output.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color::Black);

    bool output_done = false;    
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // Draw
        window.clear(sf::Color::Black);

        lock_mutex(&thread_control);
        for(uint32_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            for(uint32_t x = 0; x < IMAGE_WIDTH; x++)
            {
                const int ypos = IMAGE_HEIGHT - 1 - y;
                const int xpos = x;
                const Vec3* pixel = &thread_control.image.pixels[ypos * IMAGE_WIDTH + xpos];

                int r = std::min(int(255.99 * sqrt(pixel->r())), 255);
                int g = std::min(int(255.99 * sqrt(pixel->g())), 255);
                int b = std::min(int(255.99 * sqrt(pixel->b())), 255);

                output.setPixel(x, y, sf::Color(r, g, b));
            }
        }
        unlock_mutex(&thread_control);

        sf::Texture tex;
        tex.loadFromImage(output);
        sf::Sprite sprite;
        sprite.setTexture(tex);
        window.draw(sprite);

        uint32_t num_finished = 0;
        for(const SectionRenderInfo& section : thread_control.image.sections)
        {
            if(section.in_progress)
            {
                sf::RectangleShape rect(sf::Vector2f( (uint32_t) section.tile_width, 
                                                      (uint32_t) section.tile_height ));
                rect.setFillColor(sf::Color(0, 0, 0, 0.0));
                rect.setPosition (section.tile_x , IMAGE_HEIGHT - section.tile_y - section.tile_height);
                rect.setOutlineThickness(1.0);
                rect.setOutlineColor(sf::Color(255.0, 255.0, 255.0));

                window.draw(rect);
            }
            if(section.is_finished)
                num_finished++;
        }
        
        if(num_finished == thread_control.image.sections.size() && !output_done)
        {
            auto time_render_end   = high_resolution_clock::now();
            auto time_render_total = duration<double>(time_render_end - time_render_begin).count();
            std::cout << "The render took " << std::fixed << std::setprecision(2)
                      << time_render_total  << " seconds.\n";
            output_done = true;
        }

        window.display();

    }
    join_render_threads(render_threads, NUM_THREADS);

    for(Vec3& pixel : thread_control.image.pixels)
    {
        // Because .BMP file format stores image pixel colors in BGR format
        int ir = std::min(int(255.99 * sqrt(pixel.b())), 255);
        int ig = std::min(int(255.99 * sqrt(pixel.g())), 255);
        int ib = std::min(int(255.99 * sqrt(pixel.r())), 255);

        image_pixels.push_back(ir);
        image_pixels.push_back(ig);
        image_pixels.push_back(ib);

    }
    write_bmp_to_file("output.bmp", image_pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);

    // Cleanup
    for(Material* mat : materials)
        delete mat;

    return 0;
}
