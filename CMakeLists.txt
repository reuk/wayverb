cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -std=gnu++1y -msse -msse2 -msse3 ")

add_subdirectory(gtest)
add_subdirectory(lib)
add_subdirectory(common)
add_subdirectory(cmd)
add_subdirectory(mic_test)
add_subdirectory(rayverb)

enable_testing()
add_subdirectory(tests)

include(CPack)
