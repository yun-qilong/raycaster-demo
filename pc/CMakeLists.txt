cmake_minimum_required(VERSION 3.20)
project(RaycasterDemo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 本工程 = PC 产品（x86 + SDL）；共享代码位于仓库根的 src/（MCU 构建树另行引入）
set(RAYCASTER_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ---- 可移植约束辅助：产品代码红线编译选项 ----
# 无异常 / 无 RTTI / 无线程安全静态局部变量（对应 MCU 移植约束）
# 注意：只用于产品 target，测试 target 不应用（gtest 需要异常支持）
function(raycaster_portable_flags target)
    target_compile_options(${target} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-threadsafe-statics
    )
endfunction()

# ---- 选项 -------------------------------------------------------
option(RAYCASTER_BUILD_TESTS "构建单元测试" ON)

# ---- 依赖：SDL2（平台层 PC 后端）--------------------------------
find_package(SDL2 REQUIRED)

# ---- 代码生成：MapData.hpp（由 mapgen.py 从 .map 生成）------------
add_custom_command(
    OUTPUT ${RAYCASTER_REPO_ROOT}/src/generated/MapData.hpp
    COMMAND python3 ${RAYCASTER_REPO_ROOT}/scripts/mapgen.py
    DEPENDS
        ${RAYCASTER_REPO_ROOT}/scripts/mapgen.py
        ${RAYCASTER_REPO_ROOT}/tools/maps/default.map
    WORKING_DIRECTORY ${RAYCASTER_REPO_ROOT}
    COMMENT "Generating MapData.hpp from default.map"
)
add_custom_target(gen_mapdata DEPENDS ${RAYCASTER_REPO_ROOT}/src/generated/MapData.hpp)

# ---- 共享代码（与 MCU 构建树共用同一份源码，各自按需引入）------------
add_subdirectory(${RAYCASTER_REPO_ROOT}/src/core core)
add_subdirectory(${RAYCASTER_REPO_ROOT}/src/platform platform)

# ---- 主程序 -----------------------------------------------------
add_executable(raycaster
    main.cpp
    NoHeap.cpp
)
target_include_directories(raycaster PRIVATE ${RAYCASTER_REPO_ROOT}/src)
target_link_libraries(raycaster PRIVATE core platform_sdl)
raycaster_portable_flags(raycaster)
add_dependencies(raycaster gen_mapdata)

# ---- 测试 -------------------------------------------------------
if(RAYCASTER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
