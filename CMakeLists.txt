# === 全局配置（只写一次）===
cmake_minimum_required(VERSION 3.21)
project(GAMES101)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(pa0/pa0)
add_subdirectory(Assignment1/code)
add_subdirectory(Assignment2/code)
add_subdirectory(Assignment3/Code)
add_subdirectory(Assignment4/code)
add_subdirectory(Assignment5/Code)
add_subdirectory(PA6/PA6/Assignment6)
add_subdirectory(PA7-1/PA7/Assignment7)
add_subdirectory(Assignment8)
# ... 其他作业