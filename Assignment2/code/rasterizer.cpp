// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <climits>

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    
    Eigen::Vector3f p(x, y, 0.0f);
    // 计算三角形的边向量
    Eigen::Vector3f ab = _v[1] - _v[0];
    Eigen::Vector3f bc = _v[2] - _v[1];
    Eigen::Vector3f ca = _v[0] - _v[2];
    // 计算点到三角形顶点的向量
    Eigen::Vector3f ap = p - _v[0];
    Eigen::Vector3f bp = p - _v[1];
    Eigen::Vector3f cp = p - _v[2];
    // 计算叉积的 z 分量, 因为三角形是逆时针构建的，所以 点p 在三角形内部相当于叉积是正反向
    return (ab.cross(ap).z() >= 0) && (bc.cross(bp).z() >= 0) && (ca.cross(cp).z() >= 0);
    //return (ap * ab).z() >= 0 && (bp * bc).z() >= 0 && (cp * ca).z() >= 0;

}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float dx[] = {0.25f, 0.75f, 0.25f, 0.75f};
    float dy[] = {0.25f, 0.25f, 0.75f, 0.75f};

	float x_max = FLT_MIN, x_min = FLT_MAX, y_max = FLT_MIN, y_min = FLT_MAX;
    for (int i = 0; i < v.size(); ++i) {
		x_max = std::max(x_max, v[i].x());
		x_min = std::min(x_min, v[i].x());
		y_max = std::max(y_max, v[i].y());
		y_min = std::min(y_min, v[i].y());
    }
    // 将包围盒边界限制在 [0, width-1] 和 [0, height-1] 之间，并转为整数
    int min_x = std::max(0, (int)std::floor(x_min));
    int max_x = std::min(width - 1, (int)std::floor(x_max));
    int min_y = std::max(0, (int)std::floor(y_min));
    int max_y = std::min(height - 1, (int)std::floor(y_max));

    for (int i = min_x; i <= max_x; ++i) {
        for (int j = min_y; j <= max_y; ++j) {
            for (int k = 0; k < 4; ++k) {
                float subi = i + dx[k];
                float subj = j + dy[k];

                if (insideTriangle(subi, subj, t.v)) {
                    auto[alpha, beta, gamma] = computeBarycentric2D(subi, subj, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;

                    // z_buffer
                    // 外层 index
                    int out_index = get_index(i, j);
                    // k相当于内层 index
                    if (depth_buf[out_index][k] > z_interpolated) {
                        depth_buf[out_index][k] = z_interpolated;
                        // 更新子采样点颜色
                        sample_color_buf[out_index][k] = t.getColor();

                        // 重新计算该像素4个子采样点的平均颜色，并写入 frame_buf
                        Eigen::Vector3f avg_color = (sample_color_buf[out_index][0] +
                            sample_color_buf[out_index][1] +
                            sample_color_buf[out_index][2] +
                            sample_color_buf[out_index][3]) / 4.0f;

                        set_pixel(Eigen::Vector3f(i, j, 1.0f), avg_color);
                    }
                }
            }
        }
    }
    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f::Zero());

        // 新增：清理子采样颜色缓存
        std::array<Eigen::Vector3f, 4> black = { Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero() };
        std::fill(sample_color_buf.begin(), sample_color_buf.end(), black);
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
        float inf = std::numeric_limits<float>::infinity();
        std::array<float, 4> inf_arr = { inf, inf, inf, inf };
        std::fill(depth_buf.begin(), depth_buf.end(), inf_arr);
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    sample_color_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    // old index: 
    //auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on