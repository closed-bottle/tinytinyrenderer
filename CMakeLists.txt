cmake_minimum_required(VERSION 3.22.1)
project(SwRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug.
#set(USE_SIMD 1)

if(NOT USE_SIMD)
    set(USE_SIMD 0)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CMakePrintHelpers)
    add_definitions(-DDEBUG=1)
endif()

if (USE_SIMD EQUAL 1)
    add_definitions(-DUSE_SIMD=1)
endif()




add_subdirectory(renderUtils)
add_subdirectory(thirdparty/special-lamp)
add_executable(SwRenderer main.cpp)
target_link_libraries(SwRenderer PRIVATE renderUtils)
target_include_directories(SwRenderer PRIVATE thirdparty)
target_include_directories(renderUtils PRIVATE thirdparty)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(USE_SIMD)
        target_compile_options(SwRenderer PRIVATE -mavx2 -mfma -static-libstdc++ -static-libgcc)
    endif()
endif()