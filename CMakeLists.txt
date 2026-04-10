cmake_minimum_required(VERSION 3.30)
project(tinytinyrenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CMakePrintHelpers)
    add_definitions(-DDEBUG=1)
endif()

add_subdirectory(geometryFormats)
add_subdirectory(fileFormats)
add_subdirectory(lineAlgorithm)
add_subdirectory(math)
add_subdirectory(triangleAlgorithm)

add_executable(tinytinyrenderer main.cpp)
target_link_libraries(tinytinyrenderer PRIVATE geometryFormats fileFormats math)
#target_link_libraries(tinytinyrenderer PRIVATE geometryFormats fileFormats lineAlgorithm math triangleAlgorithm)
