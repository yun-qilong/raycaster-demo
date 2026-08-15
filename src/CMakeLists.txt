add_subdirectory(core)
add_subdirectory(platform)

# ---- 主程序：hello window ----
add_executable(raycaster
    main.cpp
    NoHeap.cpp
)
target_include_directories(raycaster PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(raycaster PRIVATE core platform_sdl)
raycaster_portable_flags(raycaster)
