
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(BezierCurve main.cpp)

target_link_libraries(BezierCurve PRIVATE ${OpenCV_LIBS})
target_link_libraries(BezierCurve PRIVATE Eigen3::Eigen)