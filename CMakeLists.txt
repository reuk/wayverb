cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_subdirectory(lib)
add_subdirectory(cmd)
add_subdirectory(tetrahedral)

enable_testing()
add_subdirectory(tests)

include(CPack)
