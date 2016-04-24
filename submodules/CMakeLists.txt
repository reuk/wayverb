cmake_minimum_required(VERSION 3.0)

add_subdirectory(gflags)
add_subdirectory(googletest)

option(WITH_GFLAGS "" OFF)
add_subdirectory(glog)

option(ASSIMP_BUILD_TESTS "" OFF)
add_subdirectory(assimp)
