cmake_minimum_required(VERSION 3.20)
project(RaycasterDemo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# ---- 子目录 -----------------------------------------------------
add_subdirectory(src)

if(RAYCASTER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
