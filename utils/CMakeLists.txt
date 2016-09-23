cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/submodules/cereal/include
)

add_subdirectory(boundary_test)
add_subdirectory(mic_test)
add_subdirectory(solution_growth)
add_subdirectory(write_compensation_signal)
add_subdirectory(southern2013_2_cuboid)
add_subdirectory(sheaffer2014)
add_subdirectory(waveguide_distance_test)
add_subdirectory(southern2011)
