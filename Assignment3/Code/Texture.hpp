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

};
#endif //RASTERIZER_TEXTURE_H
