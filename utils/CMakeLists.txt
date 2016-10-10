cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

#include_directories(
#    ${CMAKE_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${WAYVERB_BINARY_DIR}/include
#    ${CMAKE_SOURCE_DIR}/common/include
#    ${CMAKE_SOURCE_DIR}/raytracer/include
#    ${CMAKE_SOURCE_DIR}/waveguide/include
#)

add_subdirectory(boundary_test)
add_subdirectory(mic_test)
add_subdirectory(solution_growth)
add_subdirectory(sheaffer2014)
add_subdirectory(waveguide_distance_test)
add_subdirectory(southern2011)
add_subdirectory(siltanen2013)
add_subdirectory(box)
