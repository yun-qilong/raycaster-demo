# utils：通用 C++ 工具（零项目依赖，最底层共享单元）
# 目前 header-only（如 CrtpBase）；将来若有 .cpp 再改为 STATIC。
add_library(utils INTERFACE)
# 位置无关：include 根 = src/（相对本文件），任何构建树 add_subdirectory 本目录都成立
target_include_directories(utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
