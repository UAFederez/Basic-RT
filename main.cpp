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

void thread_render_image(ThreadImageInfo* info)
{
    for( uint32_t row = 0; row < info->image_height; row++ )
    {
        for( uint32_t col = 0; col < info->image_width; col++ )
        {
            Vector3 pixel_color = {};
            for(uint32_t i = 0; i < info->num_samples; i++)
            {
                float u = float(col + random_float()) / float(info->image_width);
                float v = float(row + random_float()) / float(info->image_height);

                Ray r  = info->scene.camera->get_ray(u, v);
                pixel_color += color(r, *info->scene.world, 0);
            }
            info->pixels.push_back(pixel_color);
            info->finished_pixels++;
        }
    }
}

int main()
{
    const int IMAGE_WIDTH  = 1280;
    const int IMAGE_HEIGHT = 720;

    std::vector<uint8_t>   image_pixels;
    std::vector<Material*> materials;

    image_pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    // Materials
    materials.push_back(new Metal(Vector3(0.8, 0.8, 0.8), 0.0 ));
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
            if(mat_prob < 0.40)
            {
                mat = new Lambertian(Vector3(0.5 + random_float() * 0.5, 
                                             0.5 + random_float() * 0.5, 
                                             0.5 + random_float() * 0.5));
            } else if(mat_prob < 0.90)
            {
                mat = new Metal(Vector3(0.5 + random_float() * 0.5, 
                                        0.5 + random_float() * 0.5, 
                                        0.5 + random_float() * 0.5),
                                        0.1);
            } else
                mat = new Dielectric(1.5);

            const float theta  = (360.0 / (8.0 * (j + 1))) * i + (j * 10);
            const float x_pos = cos(theta * M_PI / 180.0f) * ring_radius;
            const float z_pos = sin(theta * M_PI / 180.0f) * ring_radius;

            materials.push_back(mat);
            scene.objects.push_back(new Sphere(Vector3(x_pos, 0.5, z_pos), 0.5, mat));
        }
        ring_radius += 2.0f;
    }

    // Camera description
    const float view_rot = 90.0f;
    const float dist     = 16.0f; // 8
    Camera main_camera(Vector3(cos(view_rot * M_PI / 180) * dist, 3, // 3 
                               sin(view_rot * M_PI / 180) * dist), 
                       Vector3( 0,0.5, 0),
                       Vector3( 0,1.0, 0),
                       30.0f, // 30
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // Image rendering description
    const uint32_t MAX_THREADS        = 12; 
    const uint32_t NUM_SAMPLES        = 72;
    const float    NS_DENOM           = 1 / float(NUM_SAMPLES);

    const uint32_t NUM_THREADS        = std::min<uint32_t>(MAX_THREADS, NUM_SAMPLES);
    const uint32_t SAMPLES_PER_THREAD = (NUM_SAMPLES / NUM_THREADS);

    printf("---------------------------\n");
    printf("Total threads       %d\n", NUM_THREADS);
    printf("Total Samples       %d\n", NUM_SAMPLES);
    printf("Samples per thread: %d\n", SAMPLES_PER_THREAD);
    printf("Additional samples: %d\n", NUM_SAMPLES % NUM_THREADS);
    printf("---------------------------\n");


    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::seconds;
    
    auto begin = high_resolution_clock::now();

    ThreadHandle    thread_handles[NUM_THREADS];
    ThreadImageInfo image_info[NUM_THREADS];

    // If NUM_SAMPLES is not divisible by NUM_THREADS, split the remaining
    // with 1 for each of the threads
    uint32_t additional_samples = NUM_SAMPLES % NUM_THREADS;

    // Initialize the jobs to do for each thread
    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        image_info[i].thread_id       = 0;
        image_info[i].num_samples     = SAMPLES_PER_THREAD;
        image_info[i].image_width     = IMAGE_WIDTH;
        image_info[i].image_height    = IMAGE_HEIGHT;
        image_info[i].finished_pixels = 0;

        image_info[i].scene.camera    = &main_camera;
        image_info[i].scene.world     = &scene;

        if(additional_samples != 0)
        {
            image_info[i].num_samples++;
            additional_samples--;
        }
    }

    if(create_render_threads(image_info, thread_handles, NUM_THREADS))
    {
        printf("[ERROR] Could not initialize all the threads. Exiting now...\n");
        return 0;
    }
    printf("[SUCCESS] All threads were successfully initialized.\n");

    const unsigned NUM_BAR_CHARS = 80;
    char  progress_bar_chars[NUM_BAR_CHARS];

    memset(progress_bar_chars, ' ', NUM_BAR_CHARS);
    while(true)
    {
        uint32_t total_finished = 0;
        uint32_t total_to_do    = 0;

        for(const ThreadImageInfo& info : image_info)
        {
            total_finished += info.finished_pixels;
            total_to_do    += info.image_width * info.image_height;
        }

        float pct = float(total_finished) / float(total_to_do);
        int nbars = pct * NUM_BAR_CHARS;

        memset(progress_bar_chars, '#', nbars);
        printf("Rendering image... [%s] [%.2f]\r", progress_bar_chars, pct * 100.0f);

        if(pct == 1.0)
        {
            printf("\n");
            break;
        }
    }
    join_threads(thread_handles, NUM_THREADS);

    auto end     = high_resolution_clock::now();
    auto elapsed = duration<double>(end - begin).count();
    printf("The render took %.2f seconds to complete.\n", elapsed);

    // Collect all the pixels from each thread and average the result
    std::vector<Vector3> total_pixels(IMAGE_WIDTH * IMAGE_HEIGHT);
    for(const ThreadImageInfo& info : image_info)
    {
        for(uint32_t i = 0; i < info.pixels.size(); i++)
           total_pixels[i] += info.pixels[i] * NS_DENOM;
    }

    // Prepare the image data
    for(Vector3& pixel : total_pixels)
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
