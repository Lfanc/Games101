find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Assignment1 main.cpp rasterizer.cpp Triangle.cpp)

target_link_libraries(Assignment1 PRIVATE ${OpenCV_LIBS})
target_link_libraries(Assignment1 PRIVATE Eigen3::Eigen)