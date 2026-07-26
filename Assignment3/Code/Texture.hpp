//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        // 源代码
        //auto u_img = u * width;
        //auto v_img = (1 - v) * height;
        //auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        //return Eigen::Vector3f(color[0], color[1], color[2]);
        
        //质谱ai改进的
        // 防止 uv 超出 [0, 1] 范围
        u = std::fmax(0.0f, std::fmin(1.0f, u));
        v = std::fmax(0.0f, std::fmin(1.0f, v));

        // 修改这里：映射到 0 ~ width-1 和 0 ~ height-1
        auto u_img = u * (width - 1);
        auto v_img = (1 - v) * (height - 1);

        // 取整
        int u_pixel = static_cast<int>(u_img);
        int v_pixel = static_cast<int>(v_img);

        // 获取像素颜色
        auto color = image_data.at<cv::Vec3b>(v_pixel, u_pixel);

        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    Eigen::Vector3f getColorBilinear(float u, float v) {
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        // 映射到 0 ~ width-1 和 0 ~ height-1
        auto u_img = u * (width - 1);
        auto v_img = (1 - v) * (height - 1);

        
        float u00 = std::floor(u_img);
        float u01 = std::ceil(u_img);
        float v00 = std::floor(v_img);
        float v01 = std::ceil(v_img);

        // 防越界，后面感觉不需要，因为前面已经映射到 0 ~ width-1 和 0 ~ height-1 这个范围了
        //u00 = std::clamp(u00, 0, width - 1);
        //u01 = std::clamp(u01, 0, width - 1);
        //v00 = std::clamp(v00, 0, height - 1);
        //v01 = std::clamp(v01, 0, height - 1);
        
        // 
        float u_floor = u_img - u00;
        float u_ceil = u_img - u01;
        if (u_floor <= u_ceil && u00 != 0.0f) {
            u01 = u00 - 1.0f;
            float temp = u00;
            u00 = u01;
            u01 = temp;
        }


        float v_floor = v_img - v00;
        float v_ceil = v_img - v01;
        if (v_floor <= v_ceil && v00 != 0.0f) {
            v01 = v00 - 1.0f;
            float temp = v00;
            v00 = v01;
            v01 = temp;
        }

        u00 = static_cast<int>(u00);
        u01 = static_cast<int>(u01);
        v00 = static_cast<int>(v00);
        v01 = static_cast<int>(v01);

        // 获取邻近 4 个texture颜色
        auto color00 = image_data.at<cv::Vec3b>(v00, u00);
        auto color01 = image_data.at<cv::Vec3b>(v00, u01);
        auto color10 = image_data.at<cv::Vec3b>(v01, u00);
        auto color11 = image_data.at<cv::Vec3b>(v01, u01);

        // u 轴 线性插值
        float u_ratio = u_img - u00;
        auto color0 = color00 + u_ratio * (color01 - color00);
        auto color1 = color10 + u_ratio * (color11 - color10);

        // v 轴 线性插值
        float v_ratio = v_img - v00;
        auto return_color = color0 + v_ratio * (color1 - color0);

        return Eigen::Vector3f(return_color[0], return_color[1], return_color[2]);
    }
};
#endif //RASTERIZER_TEXTURE_H
