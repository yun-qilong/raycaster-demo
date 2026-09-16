# core：平台无感核心（renderer/math/container/logic）
# header-only（目前），零平台宏。编译选项由各产品 target 自行加红线。
add_library(core INTERFACE)
# 位置无关：include 根 = src/（相对本文件），任何构建树 add_subdirectory 本目录都成立
target_include_directories(core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
