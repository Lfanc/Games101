// main.cpp
// 简要说明：
// 这是一个用于展示简易光栅化流程的示例程序。程序加载一个三角形的顶点数据，
// 并通过设置模型（model）、视图（view）和投影（projection）矩阵将三角形渲染到帧缓冲，
// 最后使用 OpenCV 将帧缓冲显示或保存为图像文件。

#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926; // PI 常量

// 计算视图矩阵（View Matrix）
// 输入：观察者位置 eye_pos（世界坐标系）
// 返回：将世界坐标系变换到相机坐标系的 4x4 矩阵
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  // 平移矩阵：将世界原点平移到相机位置的逆向
  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2],
	  0, 0, 0, 1;

  // 视图矩阵为平移矩阵与单位矩阵的乘积（这里只需平移）
  view = translate * view;

  return view;
}

// 计算模型矩阵（Model Matrix）
// 输入：绕 Z 轴的旋转角度 rotation_angle（度）
// 返回：将模型从模型空间变换到世界空间的 4x4 矩阵
Eigen::Matrix4f get_model_matrix(float rotation_angle) {
  Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

  // 将角度转换为弧度
  float radian = rotation_angle / 180 * MY_PI;

  // 绕 Z 轴旋转的齐次矩阵（右手坐标系，逆时针为正）
  model << cos(radian), -sin(radian), 0, 0,
			sin(radian),  cos(radian), 0, 0,
			0,             0,            1, 0,
			0,             0,            0, 1;

  return model;
}

// 绕任意过原点的轴旋转（罗德里格旋转公式）
// axis: 旋转轴方向（不要求是单位向量）
// angle: 旋转角度（单位：度，与作业中其他函数保持一致）
Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float angle) {
	// 1. 角度转弧度
	float rad = angle / 180.0f * MY_PI;

	// 2. 将旋转轴归一化为单位向量
	axis.normalize();

	// 3. 构造轴对应的反对称矩阵 K（叉积矩阵）
	Eigen::Matrix3f K;
	K << 0, -axis(2), axis(1),
		axis(2), 0, -axis(0),
		-axis(1), axis(0), 0;

	// 4. 罗德里格公式：R = I + sin(θ)·K + (1 - cos(θ))·K²
	Eigen::Matrix3f R = Eigen::Matrix3f::Identity()
		+ std::sin(rad) * K
		+ (1 - std::cos(rad)) * K * K;

	// 5. 扩展为 4x4 齐次变换矩阵（最后一行为 0 0 0 1）
	Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
	result.block<3, 3>(0, 0) = R;
	return result;
}

// 计算投影矩阵（Projection Matrix）
// 输入：视野角 eye_fov（度，竖直视野角度），宽高比 aspect_ratio，近裁剪面 zNear，远裁剪面 zFar
// 返回：用于将视图坐标投影到裁剪空间（NDC）的 4x4 矩阵
// NOTE: 本函数目前保留为学生实现位置，函数体内仍然返回单位矩阵；
//       可以在此处实现透视投影（透视->正交的转换）以完成完整渲染管线。
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
						  float zNear, float zFar) {
  // Students will implement this function

  Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

  // TODO: Implement this function
  // Create the projection matrix for the given parameters.
  // Then return it.
  Eigen::Matrix4f persp2ortho;
  persp2ortho << -zNear, 0, 0, 0,
				  0, -zNear, 0, 0,
				  0, 0, -zNear - zFar, -zNear * zFar,
				  0, 0, 1, 0;

  float t = tan(eye_fov / 2 * MY_PI / 180) * zNear;
  float r = t * aspect_ratio;
  float l = -r;
  float b = -t;
  Eigen::Matrix4f ortho, ortho_S, ortho_T;
  ortho_T << 1, 0, 0, -(r+l) / 2,
			  0, 1, 0, -(b+t) / 2,
			  0, 0, 1, (zNear + zFar) / 2,
			  0, 0, 0, 1;
  ortho_S << 2 / (r - l), 0, 0, 0,
			  0, 2 / (t - b), 0, 0,
			  0, 0, 2 / (zFar - zNear), 0,
			  0, 0, 0, 1;
  ortho = ortho_S * ortho_T;
  projection = ortho * persp2ortho;
  return projection;
}

int main(int argc, const char **argv) {
  float angle = 0;                // 模型旋转角度（度）
  bool command_line = false;      // 是否以命令行模式运行（非交互）
  std::string filename = "output.png"; // 命令行模式下输出文件名

  // 解析命令行参数：如果传入参数则进入命令行模式并读取旋转角度和文件名
  if (argc >= 3) {
	command_line = true;
	angle = std::stof(argv[2]); // -r by default
	if (argc == 4) {
	  filename = std::string(argv[3]);
	} else
	  return 0;
  }

  // 创建光栅化器，帧缓冲大小 700x700
  rst::rasterizer r(700, 700);

  // 相机位置（世界坐标系）
  Eigen::Vector3f eye_pos = {0, 0, 5};

  // 三角形顶点（世界坐标系）
  std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

  // 三角形索引（单个三角形）
  std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

  // 将顶点和索引加载到光栅化器并获得对应 id
  auto pos_id = r.load_positions(pos);
  auto ind_id = r.load_indices(ind);

  int key = 0;
  int frame_count = 0;

  // 命令行模式：渲染一帧并保存到文件
  if (command_line) {
	r.clear(rst::Buffers::Color | rst::Buffers::Depth);

	//r.set_model(get_model_matrix(angle));
	// 新代码：绕过原点的 (1,1,1) 方向轴旋转
	Eigen::Vector3f axis(1.0f, 1.0f, 1.0f);
	r.set_model(get_rotation(axis, angle));
	r.set_view(get_view_matrix(eye_pos));
	r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

	r.draw(pos_id, ind_id, rst::Primitive::Triangle);
	cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
	image.convertTo(image, CV_8UC3, 1.0f);

	cv::imwrite(filename, image);

	return 0;
  }

  // 交互模式：循环渲染并显示，按 'a'/'d' 控制角度，按 ESC 退出
  while (key != 27) {
	r.clear(rst::Buffers::Color | rst::Buffers::Depth);

	//r.set_model(get_model_matrix(angle));
	// 新代码：绕过原点的 (1,1,1) 方向轴旋转
	Eigen::Vector3f axis(0.0f, 0.0f, 1.0f);
	r.set_model(get_rotation(axis, angle));
	r.set_view(get_view_matrix(eye_pos));
	r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

	r.draw(pos_id, ind_id, rst::Primitive::Triangle);

	cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
	image.convertTo(image, CV_8UC3, 1.0f);
	cv::imshow("image", image);
	key = cv::waitKey(10);

	std::cout << "frame count: " << frame_count++ << '\n';

	if (key == 'a') {
	  angle += 10;
	} else if (key == 'd') {
	  angle -= 10;
	}
  }

  return 0;
}
