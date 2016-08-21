cmake_minimum_required(VERSION 3.0)

project(WAYVERB VERSION 0.1 LANGUAGES CXX)

set(ONLY_BUILD_DOCS false CACHE BOOL "skip configuring the build and just build docs")

if(ONLY_BUILD_DOCS)
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
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    set(CMAKE_BUILD_TYPE RelWithDebInfo)

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/submodules/hana/cmake")

    add_subdirectory(submodules)

    enable_testing()
    add_subdirectory(common)
    add_subdirectory(waveguide)
    add_subdirectory(raytracer)
    add_subdirectory(combined)
    #add_subdirectory(tests)
    add_subdirectory(data)
    add_subdirectory(utils)

    include(CPack)
endif()
