# core：平台无感核心（renderer/math/container/logic）
# header-only（目前），零平台宏。编译选项由各产品 target 自行加红线。
add_library(core INTERFACE)
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR}/src)
