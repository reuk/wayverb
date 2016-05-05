cmake_minimum_required(VERSION 3.0)

project(WAYVERB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(submodules)

enable_testing()
add_subdirectory(common)
add_subdirectory(waveguide)
add_subdirectory(raytracer)
add_subdirectory(tests)

include(CPack)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc #ALL
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR})
endif(DOXYGEN_FOUND)

add_custom_target(compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
