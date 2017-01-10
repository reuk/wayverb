cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

project(WAYVERB VERSION 0.1 LANGUAGES CXX)

set(ONLY_BUILD_DOCS false CACHE BOOL "skip configuring the build and just build docs")

if(ONLY_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(doc ALL
            ${DOXYGEN_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR})
    endif(DOXYGEN_FOUND)
else()
    set(CMAKE_VERBOSE_MAKEFILE ON)

    include(${CMAKE_CURRENT_SOURCE_DIR}/config/dependencies.cmake)

    set(CMAKE_BUILD_TYPE RelWithDebugInfo)

    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -ftemplate-depth=1024)

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

    #set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    #set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

    enable_testing()
    add_subdirectory(src)
    add_subdirectory(bin)
endif()
