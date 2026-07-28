

add_executable(RayTracing main.cpp Object.hpp Vector.hpp Sphere.hpp global.hpp Triangle.hpp Scene.cpp Scene.hpp Light.hpp Renderer.cpp)

# 根据编译器类型设置不同的编译选项
if(MSVC)
    # MSVC 使用 /W4 开启较高警告级别（类似 -Wall），/WX- 表示不将警告视为错误
    target_compile_options(RayTracing PUBLIC /W4 /WX-)
    # 移除所有不支持的 GCC/Clang 选项，包括 -fsanitize
else()
    # GCC / Clang 使用原有的严格警告和未定义行为检测
    target_compile_options(RayTracing PUBLIC -Wall -Wextra -pedantic -Wshadow -Wreturn-type -fsanitize=undefined)
    # 链接时也需要加上 -fsanitize=undefined
    target_link_libraries(RayTracing PUBLIC -fsanitize=undefined)
endif()

# 统一要求 C++17 标准（MSVC 也支持）
target_compile_features(RayTracing PUBLIC cxx_std_17)
