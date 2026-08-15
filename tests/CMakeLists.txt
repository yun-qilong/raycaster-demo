include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

add_executable(raycaster_ut
    TestFixed.cpp
)
target_include_directories(raycaster_ut PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(raycaster_ut PRIVATE core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(raycaster_ut PROPERTIES LABELS "raycaster")
