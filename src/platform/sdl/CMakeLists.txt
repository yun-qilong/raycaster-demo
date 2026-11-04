# SDL 后端（PC 专属）：使用方（PC 树）负责 find_package(SDL2)
add_library(platform_sdl STATIC
    PcPlant.cpp
    PcPlant.hpp
)
# 位置无关：include 根 = src/（相对本文件）
target_include_directories(platform_sdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(platform_sdl PUBLIC SDL2::SDL2)
raycaster_portable_flags(platform_sdl)
