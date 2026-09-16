# platform：平台层（PC = SDL 后端；未来 MCU = 新后端）
# 平台接口（api）零平台宏；平台宏（#ifdef）只允许出现在各后端内部。

# ---- 平台接口（header-only）----
add_library(platform_api INTERFACE)
target_include_directories(platform_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ---- SDL 后端（PC）----
add_library(platform_sdl STATIC
    sdl/SdlPlatform.cpp
    sdl/SdlPlatform.hpp
)
target_include_directories(platform_sdl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(platform_sdl PUBLIC platform_api SDL2::SDL2)
raycaster_portable_flags(platform_sdl)
