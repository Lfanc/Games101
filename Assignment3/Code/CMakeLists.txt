
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)


include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Assignment3 main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp Texture.hpp Texture.cpp Shader.hpp OBJ_Loader.h)
target_link_libraries(Assignment3 PRIVATE ${OpenCV_LIBS})
target_link_libraries(Assignment3 PRIVATE Eigen3::Eigen)
#target_compile_options(Rasterizer PUBLIC -Wall -Wextra -pedantic)