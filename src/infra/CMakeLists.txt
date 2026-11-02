# infra：与具体游戏无关的基础设施（定点数学、地图、碰撞、渲染、循环）
# header-only（目前），零平台宏。编译选项由各产品 target 自行加红线。
add_library(infra INTERFACE)
# 位置无关：include 根 = src/（相对本文件），任何构建树 add_subdirectory 本目录都成立
target_include_directories(infra INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
