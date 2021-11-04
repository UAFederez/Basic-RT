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

#include "util/BitmapImage.h"
#include "util/Threading.h"
#include "util/General.h"

#include "math/Vector3.h"
#include "math/Ray.h"

#include "graphics/Material.h"
#include "graphics/Camera.h"
#include "graphics/Sphere.h"
#include "graphics/Triangle.h"
#include "graphics/Plane.h"
#include "graphics/World.h"

Vector3 rotate_y(const Vector3& v, const float theta)
{
    const float rad = theta * M_PI / 180.0f;
    return Vector3( dot(v, Vector3(cos(rad), 0, -sin(rad))),
                    dot(v, Vector3(0, 1, 0)),
                    dot(v, Vector3(sin(rad), 0, cos(rad))));
                                   
}

Vector3 color(const Ray& r, const World& world, int depth)
{
    HitRecord rec = {};
    if(world.hit_anything(r, 1e-3, FLT_MAX, rec))
    {
        Ray     scattered;
        Vector3 attenuation;
        
        if(depth > 50)
           return Vector3(0.0, 0.0, 0.0); 
        
        Vector3 emitted = rec.material_ptr->emitted();

        if(rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return emitted + attenuation * color(scattered, world, depth + 1);

        return emitted;
    }
    return Vector3(0.010, 0.010, 0.035); // TODO: add ambient color to world parameters
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
                    Vector3 pixel = {};
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

int main()
{
    // Materials
    std::vector<Material*> materials;
    materials.push_back(new Metal(Vector3(0.5, 0.5, 0.5), 0.10));
    materials.push_back(new Metal(Vector3(0.9, 0.8, 0.8), 0.05));
    materials.push_back(new Dielectric(1.3));
    materials.push_back(new Lambertian(Vector3(0.8, 0.8, 0.9)));
    materials.push_back(new Emissive(Vector3(1.0, 1.0, 1.0)));
    materials.push_back(new Emissive(Vector3(2.5, 2.5, 1.5)));

    // Scene objects
    World scene = {};
    
    const float HALF_SIDE = 1.1f;
    const float height    = 4.0f;

    const auto  transform = [](const Vector3& v, const float theta) 
    {
        const Vector3 anchor = Vector3(0.0, 0.0, -10);
        Vector3 translated   = v + anchor;
        Vector3 rotated      = rotate_y(translated, theta);
        Vector3 reverted     = rotated - anchor;

        return reverted;
    };
    float theta = -30;
    
    for(int i = 0; i < 5; i++)
    {
        Material* mat = new Emissive(Vector3(0.7 + random_float(0.0, 0.2),
                                             0.7 + random_float(0.0, 0.2),
                                             0.7 + random_float(0.0, 0.2)));

        scene.objects.push_back(new Triangle(transform(Vector3(-HALF_SIDE, 0.0, 0.0), theta),
                                             transform(Vector3( HALF_SIDE, 0.0, 0.0), theta),
                                             transform(Vector3( HALF_SIDE, height, 0.0), theta),
                                             mat));

        scene.objects.push_back(new Triangle(transform(Vector3(-HALF_SIDE, 0.0, 0.0), theta),
                                             transform(Vector3( HALF_SIDE, height, 0.0), theta),
                                             transform(Vector3(-HALF_SIDE, height, 0.0), theta),
                                             mat));
        materials.push_back(mat);
        theta += 15;
    }


    const float CEIL_HEIGHT = 50.0f;
    const float CEIL_HALF   = 10.0f;
    const float FLOOR_HALF  = 10.0f;

    // floor
    scene.objects.push_back(new Triangle(Vector3(-FLOOR_HALF, 0, FLOOR_HALF),
                                         Vector3( FLOOR_HALF, 0, FLOOR_HALF),
                                         Vector3( FLOOR_HALF, 0,-FLOOR_HALF * 2.0),
                                         materials[1]));
    scene.objects.push_back(new Triangle(Vector3(-FLOOR_HALF, 0, FLOOR_HALF),
                                         Vector3( FLOOR_HALF, 0,-FLOOR_HALF * 2.0),
                                         Vector3(-FLOOR_HALF, 0,-FLOOR_HALF * 2.0),
                                         materials[1]));

    // Front light
    const float FRONT_HALF_H = 20.0f;
    const float FRONT_HALF_V = 20.0f;
    scene.objects.push_back(new Triangle(Vector3( FRONT_HALF_H,-FRONT_HALF_V, 50.0),
                                         Vector3(-FRONT_HALF_H,-FRONT_HALF_V, 50.0),
                                         Vector3(-FRONT_HALF_H, FRONT_HALF_V, 50.0),
                                         materials[4]));

    scene.objects.push_back(new Triangle(Vector3( FRONT_HALF_H,-FRONT_HALF_V, 50.0),
                                         Vector3(-FRONT_HALF_H, FRONT_HALF_V, 50.0),
                                         Vector3( FRONT_HALF_H, FRONT_HALF_V, 50.0),
                                         materials[4]));


    scene.objects.push_back(new Triangle(Vector3(-CEIL_HALF, CEIL_HEIGHT,-CEIL_HALF),
                                         Vector3( CEIL_HALF, CEIL_HEIGHT,-CEIL_HALF),
                                         Vector3( CEIL_HALF, CEIL_HEIGHT, CEIL_HALF * 2.0),
                                         materials[4]));
    scene.objects.push_back(new Triangle(Vector3(-CEIL_HALF, CEIL_HEIGHT,-CEIL_HALF),
                                         Vector3( CEIL_HALF, CEIL_HEIGHT, CEIL_HALF * 2.0),
                                         Vector3(-CEIL_HALF, CEIL_HEIGHT, CEIL_HALF * 2.0),
                                         materials[4]));

    scene.objects.push_back(new Sphere(Vector3(  0.0, 0.75, 3.0), 0.75, materials[2]));
    scene.objects.push_back(new Sphere(Vector3(  3.0, 0.75, 3.0), 0.75, materials[2]));
    scene.objects.push_back(new Sphere(Vector3( -3.0, 0.75, 3.0), 0.75, materials[2]));
    scene.objects.push_back(new Sphere(Vector3( 1.75, 1.5, 1.0), 1.5, materials[0]));
    scene.objects.push_back(new Sphere(Vector3(-1.75, 1.5, 1.0), 1.5, materials[1]));

    // Lighting
    scene.light_position = Vector3(0.0,5.0,1.0);

    // Image rendering description
    const uint32_t IMAGE_WIDTH  = 1280;
    const uint32_t IMAGE_HEIGHT = 720;
    const uint32_t TILE_WIDTH   = 16;
    const uint32_t TILE_HEIGHT  = 16;

    // Camera description
    const float view_rot = 90.0f;
    const float dist     = 15.0f; // 8
    Camera main_camera(Vector3( 0, 3.0,12),
                       Vector3( 0, 1.0, 0),
                       Vector3( 0, 1.0, 0),
                       30.0f, // 30
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // Rendering thread parameters
    const uint32_t MAX_THREADS  = 12; 
    const uint32_t NUM_SAMPLES  = 120;
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
    thread_control.image.pixels       = std::vector<Vector3>(IMAGE_WIDTH * IMAGE_HEIGHT);
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
    
    std::ofstream output("output.ppm");
    output << "P3\n" << IMAGE_WIDTH << ' ' << IMAGE_HEIGHT << '\n'
            << "255\n";
    for(Vector3& pixel : thread_control.image.pixels)
    {
        pixel = Vector3(sqrt(pixel.x()), sqrt(pixel.y()), sqrt(pixel.z()));

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
