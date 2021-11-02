#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <cfloat>
#include <time.h>
#include <sys/time.h>

#include "util/ProgressBar.h"
#include "util/BitmapImage.h"
#include "util/Threading.h"

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

void* process_thread_work(void* info_param)
{
    WorkerInfo* info = (WorkerInfo*) info_param;

    for( uint32_t row = 0; row < info->image_height; row++ )
    {
        for( uint32_t col = 0; col < info->image_width; col++ )
        {
            Vector3 pixel_color = {};
            for(uint32_t i = 0; i < info->num_samples; i++)
            {
                float u = float(col + float(rand())/float(RAND_MAX)) / float(info->image_width);
                float v = float(row + float(rand())/float(RAND_MAX)) / float(info->image_height);

                Ray r  = info->scene.camera->get_ray(u, v);
                pixel_color += color(r, *info->scene.world, 0);
            }
            info->pixels.push_back(pixel_color);
            info->finished_pixels++;
        }
    }
    return NULL;
}

int main()
{

    srand(time(0));
    const int IMAGE_WIDTH  = 1280;
    const int IMAGE_HEIGHT = 720;
    const int NUM_SAMPLES  = 12;
    const float NS_DENOM   = 1 / float(NUM_SAMPLES);

    std::vector<uint8_t>   pixels;
    std::vector<Material*> materials;

    pixels.reserve(IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    // Scene description
    World scene = {};
    scene.objects.push_back(new Plane (Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 
                                       new Metal(Vector3(0.8, 0.8, 0.8), 0.0)));
    scene.objects.push_back(new Plane (Vector3(0.0, 0.0,-4), Vector3(0.0, 0.0, 1.0), 
                                       new Metal(Vector3(0.8, 0.8, 0.8), 0.05)));
    scene.objects.push_back(new Sphere(Vector3( 0.0, 1.5, 0.0), 1.5, 
                                       new Dielectric(1.5)));

    float ring_radius = 2.0f;
    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < (9 + j) * (j + 1); i++)
        {
            Material* mat;
            float mat_prob = float(rand()) / float(RAND_MAX);
            if(mat_prob < 0.40)
            {
                mat = new Lambertian(Vector3(0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                             0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                             0.5 + float(rand())/float(RAND_MAX) * 0.5));
            } else if(mat_prob < 0.80)
            {
                mat = new Metal(Vector3(0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                        0.5 + float(rand())/float(RAND_MAX) * 0.5, 
                                        0.5 + float(rand())/float(RAND_MAX) * 0.5),
                                        0.1);
            } else
                mat = new Dielectric(1.5);

            const float theta  = (360 / ((9 + j) * (j + 1))) * i;
            const float x_pos = cos(theta * M_PI / 180.0f) * ring_radius;
            const float z_pos = sin(theta * M_PI / 180.0f) * ring_radius;

            materials.push_back(mat);
            scene.objects.push_back(new Sphere(Vector3(x_pos, 0.5, z_pos), 0.5, mat));

        }
        ring_radius *= 2;
    }

    // Camera description
    const float view_rot = 90.0f;
    const float dist     = 8.0f;
    Camera main_camera(Vector3(cos(view_rot * M_PI / 180) * dist, 3, 
                               sin(view_rot * M_PI / 180) * dist), 
                       Vector3( 0,1.0, 0),
                       Vector3( 0,1.0, 0),
                       30,
                       float(IMAGE_WIDTH) / float(IMAGE_HEIGHT));

    // For measuring the current progress
    Progress progress = {};
    progress.width    = IMAGE_WIDTH;
    progress.height   = IMAGE_HEIGHT;
    progress.finished = 0;

    //ThreadInfo thread_info;
    //create_thread(&progress, &thread_info);
    
    const uint32_t NUM_THREADS        = 12;
    const uint32_t SAMPLES_PER_THREAD = (NUM_SAMPLES / NUM_THREADS);
    WorkerInfo work_info[NUM_THREADS];
    pthread_t  threads[NUM_THREADS];

    printf("Each thread will do %d samples\n", SAMPLES_PER_THREAD);

    for(uint32_t i = 0; i < NUM_THREADS; i++)
    {
        work_info[i].thread_id       = i;
        work_info[i].num_samples     = SAMPLES_PER_THREAD;
        work_info[i].image_width     = IMAGE_WIDTH;
        work_info[i].image_height    = IMAGE_HEIGHT;
        work_info[i].finished_pixels = 0;
        work_info[i].scene.camera    = &main_camera;
        work_info[i].scene.world     = &scene;

        if(i == NUM_THREADS - 1)
            work_info[i].num_samples += NUM_SAMPLES % NUM_THREADS;

        if(pthread_create(&threads[i], NULL, process_thread_work, (void*)&work_info[i]))
            printf("[ERROR] Could not create thread #%d\n", i);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    char bar[80];
    memset(bar, ' ', 80);
    while(true)
    {
        uint32_t total_finished = 0;
        uint32_t total_to_do    = 0;

        for(const WorkerInfo& info : work_info)
        {
            total_finished += info.finished_pixels;
            total_to_do    += info.image_width * info.image_height;
        }

        float pct = float(total_finished) / float(total_to_do);
        int nbars = pct * 80;

        memset(bar, '#', nbars);
        printf("[%s] [%.2f]\r", bar, pct * 100.0f);

        if(pct == 1.0)
        {
            printf("\n");
            break;
        }
    }

    for(uint32_t i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    std::vector<Vector3> total_pixels(IMAGE_WIDTH * IMAGE_HEIGHT);
    for(const WorkerInfo& info : work_info)
    {
        for(uint32_t i = 0; i < info.pixels.size(); i++)
           total_pixels[i] += info.pixels[i] * NS_DENOM;
    }

    for(Vector3& pixel : total_pixels)
    {
        pixel = Vector3(sqrt(pixel.x()), sqrt(pixel.y()), sqrt(pixel.z()));
        pixels.push_back(uint8_t(255.99 * pixel.z()));
        pixels.push_back(uint8_t(255.99 * pixel.y()));
        pixels.push_back(uint8_t(255.99 * pixel.x()));
    }
    gettimeofday(&end, NULL);

    double time_taken = end.tv_sec + end.tv_usec / 1e6 -
                        start.tv_sec - start.tv_usec / 1e6; // in seconds

    printf("time program took %f seconds to execute\n", time_taken);

    //join_thread(&thread_info);

    write_bmp_to_file("output.bmp", pixels.data(), IMAGE_WIDTH, IMAGE_HEIGHT, 3);
    // Cleanup
    for(Material* mat : materials)
        delete mat;
    return 0;
}
