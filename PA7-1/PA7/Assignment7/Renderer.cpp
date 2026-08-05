//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include <atomic> 
#include <thread>
#include <vector>
#include <mutex>

inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    //int m = 0;
    int spp = 32;

    int totalRows = scene.height;
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 8;

    // 1. 原子计数器：记录已经渲染完成了多少行（线程安全，无需锁）
    std::atomic<int> rowsCompleted(0);
    // 2. 线程主函数（按连续行切分）
    auto renderRows = [&](int startRow, int endRow) {
        for (int j = startRow; j < endRow; ++j) {
            for (uint32_t i = 0; i < scene.width; ++i) {
                // 生成光线方向（完全照抄你的原代码）
                float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                    imageAspectRatio * scale;
                float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
                Vector3f dir = normalize(Vector3f(-x, y, 1));

                // 计算该像素索引（注意：这里不用全局 m，直接算！）
                int idx = j * scene.width + i;
                Vector3f color(0.0f);

                // 16 条光线在内部串行累加（依然无锁）
                for (int k = 0; k < spp; ++k) {
                    color += scene.castRay(Ray(eye_pos, dir), 0);
                }
                framebuffer[idx] = color / (float)spp;
            }

            // 3. 每一行干完，原子计数 +1（这一行极快，不会阻塞性能）
            int done = rowsCompleted.fetch_add(1) + 1;

            // 4. 关键优化：每渲染 10% 才更新一次进度（减少 I/O 开销）
            if (done % (totalRows / 10) == 0 || done == totalRows) {
                // 加锁只是为了保护控制台不输出乱码，但此时已经渲染了很多行，调用次数极少（仅10次）
                static std::mutex console_mtx;
                std::lock_guard<std::mutex> lock(console_mtx);
                UpdateProgress((float)done / totalRows);
            }
        }
        };

    // 5. 启动多线程
    std::vector<std::thread> threads;
    int rowsPerThread = totalRows / numThreads;
    for (int t = 0; t < numThreads; ++t) {
        int start = t * rowsPerThread;
        int end = (t == numThreads - 1) ? totalRows : start + rowsPerThread;
        threads.emplace_back(renderRows, start, end);
    }
    for (auto& th : threads) th.join();
    //// change the spp value to change sample ammount
    //int spp = 16;
    //std::cout << "SPP: " << spp << "\n";
    //for (uint32_t j = 0; j < scene.height; ++j) {
    //    for (uint32_t i = 0; i < scene.width; ++i) {
    //        // generate primary ray direction
    //        float x = (2 * (i + 0.5) / (float)scene.width - 1) *
    //                  imageAspectRatio * scale;
    //        float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

    //        Vector3f dir = normalize(Vector3f(-x, y, 1));
    //        for (int k = 0; k < spp; k++){
    //            framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
    //        }
    //        m++;
    //    }
    //    UpdateProgress(j / (float)scene.height);
    //}
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
