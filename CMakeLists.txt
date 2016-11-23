cmake_minimum_required(VERSION 3.2)

project(WAYVERB VERSION 0.1 LANGUAGES CXX)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc #ALL
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR})
endif(DOXYGEN_FOUND)

include(dependencies.txt)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -ftemplate-depth=1024)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

enable_testing()
add_subdirectory(src)
add_subdirectory(bin)

include(CPack)
