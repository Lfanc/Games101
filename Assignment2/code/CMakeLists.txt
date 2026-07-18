
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)


include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Assignment2 main.cpp rasterizer.hpp rasterizer.cpp global.hpp Triangle.hpp Triangle.cpp)
target_link_libraries(Assignment2 PRIVATE ${OpenCV_LIBS})
target_link_libraries(Assignment2 PRIVATE Eigen3::Eigen)