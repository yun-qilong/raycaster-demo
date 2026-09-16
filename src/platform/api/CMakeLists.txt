# 平台接口（header-only）：CRTP 契约 Platform<Impl>，零平台宏
add_library(platform_api INTERFACE)
# 位置无关：include 根 = src/（相对本文件）
target_include_directories(platform_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(platform_api INTERFACE core utils)
