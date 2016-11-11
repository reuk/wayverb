cmake_minimum_required(VERSION 3.1)

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
    #set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    #if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    #    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    #        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
    #endif()

    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    set(CMAKE_BUILD_TYPE RelWithDebInfo)

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

    include(dependencies.txt)

    enable_testing()
    add_subdirectory(src)
    add_subdirectory(bin)

    include(CPack)
endif()
