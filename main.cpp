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

#include "util/ProgressBar.h"
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

Vector3 color(const Ray& r, const World& world, int depth)
{
    HitRecord rec = {};
    if(world.hit_anything(r, 1e-3, FLT_MAX, rec))
    {
        Ray     scattered;
        Vector3 attenuation;
        
        if(depth < 50 && rec.material_ptr->scatter(r, rec, attenuation, scattered))
            return comp_mul(attenuation, color(scattered, world, depth + 1));
        else
            return Vector3(0.0, 0.0, 0.0);
    }
    Vector3 unit_dir = normalize(r.direction());
    float t = 0.5 * (unit_dir.y() + 1.0f);
    return (1.0 - t) * Vector3(1.0, 1.0, 1.0) + t * Vector3(0.5, 0.7, 1.0);
}

struct SectionRenderInfo
{
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t tile_x;
    uint32_t tile_y;
};

struct ImageRenderInfo
{
    std::vector<SectionRenderInfo> sections;
    std::vector<Vector3> pixels;

    pthread_mutex_t lock;
    uint32_t num_finished_sections;

    uint32_t image_width;
    uint32_t image_height;
    uint32_t num_samples;

    World*  world;
    Camera* camera;
};

void* thread_render_image_tiles(void* img_param)
{
    ImageRenderInfo* image = (ImageRenderInfo*) img_param;

    while(true)
    {
        SectionRenderInfo* current_section = NULL;

        // Check if queue is not empty,
        pthread_mutex_lock(&image->lock);

        if(image->num_finished_sections != image->sections.size())
            current_section = &image->sections[image->num_finished_sections++];

        pthread_mutex_unlock(&image->lock);

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
            current_section = NULL;
        } else
            break;

    }
    return NULL;
}

int main()
{
    // Materials
    std::vector<Material*> materials;
    materials.push_back(new Metal(Vector3(0.8, 0.8, 0.9), 0.0 ));
    materials.push_back(new Metal(Vector3(0.8, 0.8, 0.8), 0.05));
    materials.push_back(new Dielectric(1.5));

    // Scene objects
    World scene = {};
    scene.objects.push_back(new Plane(Vector3(0.0, 0.0, 0.0), 
                                      Vector3(0.0, 1.0, 0.0), 
                                      materials[0]));

    scene.objects.push_back(new Plane(Vector3(0.0, 0.0,-18.0),
                                      Vector3(0.0, 0.0,  1.0), 
                                      materials[1]));

    scene.objects.push_back(new Sphere(Vector3( 0.0, 2.0, 0.0), 2.0, 
                                       materials[2]));

    float ring_radius = 3.0f;
    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < (8 * (j + 1)); i++)
        {
            Material* mat;
            float mat_prob = random_float();
            if(mat_prob < 0.33)
            {
                mat = new Lambertian(Vector3(0.5 + random_float() * 0.5, 
                                             0.5 + random_float() * 0.5, 
                                             0.5 + random_float() * 0.5));
            } else if(mat_prob < 0.66)
            {
                mat = new Metal(Vector3(0.5 + random_float() * 0.5, 
                                        0.5 + random_float() * 0.5, 
                                        0.5 + random_float() * 0.5),
                                        0.1);
            } else
                mat = new Dielectric(1.0 + random_float());

            const float theta  = (360.0 / (8.0 * (j + 1))) * i + (j * 10);
            const float x_pos = cos(theta * M_PI / 180.0f) * ring_radius;
            const float z_pos = sin(theta * M_PI / 180.0f) * ring_radius;

            materials.push_back(mat);
            scene.objects.push_back(new Sphere(Vector3(x_pos, 0.5, z_pos), 0.5, mat));
        }
        ring_radius += 2.0f;
    }

    // Image rendering description
    const uint32_t IMAGE_WIDTH  = 1280;
    const uint32_t IMAGE_HEIGHT = 720;
    const uint32_t TILE_WIDTH   = 16;
    const uint32_t TILE_HEIGHT  = 16;

    // Camera description
    const float view_rot = 90.0f;
    const float dist     = 14.0f; // 8
    Camera main_camera(Vector3(cos(view_rot * M_PI / 180) * dist, 3, // 3 
                               sin(view_rot * M_PI / 180) * dist), 
                       Vector3( 0,1.0, 0),
                       Vector3( 0,1.0, 0),
                       45.0f, // 30
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // Rendering thread parameters
    const uint32_t MAX_THREADS  = 12; 
    const uint32_t NUM_SAMPLES  = 180;
    const uint32_t NUM_THREADS  = MAX_THREADS;

    std::vector<uint8_t>   image_pixels;

    image_pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    // TODO: assuming for now that the image can be split up into square tiles evenly
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
    
    ImageRenderInfo image_info = {};

    image_info.image_width  = IMAGE_WIDTH;
    image_info.image_height = IMAGE_HEIGHT;
    image_info.num_samples  = NUM_SAMPLES;
    image_info.world        = &scene;
    image_info.camera       = &main_camera;
    image_info.lock         = PTHREAD_MUTEX_INITIALIZER;
    image_info.pixels       = std::vector<Vector3>(IMAGE_WIDTH * IMAGE_HEIGHT);
    image_info.num_finished_sections = 0;

    for(uint32_t tile_y = 0; tile_y < HEIGHT_IN_TILES; tile_y++)
    {
        for(uint32_t tile_x = 0; tile_x < WIDTH_IN_TILES; tile_x++)
        {
            SectionRenderInfo section = {};
            section.tile_width  = TILE_WIDTH;
            section.tile_height = TILE_HEIGHT;
            section.tile_x      = tile_x * TILE_WIDTH;
            section.tile_y      = tile_y * TILE_HEIGHT;

            if(tile_x == WIDTH_IN_TILES - 1)
                section.tile_width  += ADDITIONAL_W;
            if(tile_y == HEIGHT_IN_TILES - 1)
                section.tile_height += ADDITIONAL_H;

            image_info.sections.push_back(section);


        }
    }

    pthread_t render_threads[NUM_THREADS];

    std::cout << "[INFO] Creating threads...\n";
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_create(&render_threads[i],
                          NULL,
                          thread_render_image_tiles,
                          (void*) &image_info))
        {
            std::cout << "[ERROR] could not create thread #" << i << ".Exiting now...\n";
            return -1;
        }
    }
    std::cout << "[SUCCESS] All threads created successfully!\n";

    const uint32_t BAR_WIDTH = 81;
    char  progress_bar[BAR_WIDTH];
    
    progress_bar[BAR_WIDTH - 1] = '\0';
    memset(progress_bar,' ',BAR_WIDTH - 1);
    while(true)
    {
        const uint32_t finished = image_info.num_finished_sections;
        const uint32_t total    = image_info.sections.size();
        const uint32_t num_bars = (float(finished) / float(total)) * (BAR_WIDTH - 1);
        memset(progress_bar,'#', num_bars);

        std::cout << "Rendering image... [" << progress_bar << "]" 
                  << "[" << finished << "/" << total << "]" << '\r';

        if(finished == total)
            break;
    }
    for(uint32_t i = 0; i < NUM_THREADS; i++)
        pthread_join(render_threads[i], NULL);

    for(Vector3& pixel : image_info.pixels)
    {
        pixel = Vector3(sqrt(pixel.x()), sqrt(pixel.y()), sqrt(pixel.z()));

        image_pixels.push_back(uint8_t(255.99 * pixel.z()));
        image_pixels.push_back(uint8_t(255.99 * pixel.y()));
        image_pixels.push_back(uint8_t(255.99 * pixel.x()));
    }

    write_bmp_to_file("output.bmp", image_pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);

    // Cleanup
    for(Material* mat : materials)
        delete mat;

    return 0;
}
