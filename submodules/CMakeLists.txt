cmake_minimum_required(VERSION 3.0)

add_subdirectory(gflags)
add_subdirectory(googletest)

option(WITH_GFLAGS "" OFF)
add_subdirectory(glog)

option(ASSIMP_BUILD_TESTS "" OFF)
add_subdirectory(assimp)

add_subdirectory(glm)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm CACHE PATH "path to glm")
add_subdirectory(modern_gl_utils)

add_subdirectory(hiss)
