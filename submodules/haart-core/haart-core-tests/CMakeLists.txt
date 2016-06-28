cmake_minimum_required(VERSION 3.0)

project(HAART_CORE_TESTS VERSION 0.1 LANGUAGES CXX)

add_definitions(-DTEST_FILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

set(CATCH_PATH "" CACHE PATH "The path to the Catch headers")

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h" "*.hpp")

if(NOT "${CATCH_PATH}" STREQUAL "")
    set(name haart-core-tests)

    add_executable(${name} ${sources} ${headers})

    target_include_directories(${name}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CATCH_PATH}
    )

    target_link_libraries(${name} haart-core)

    enable_testing()
    #add_test(haart-core ${name})
endif(NOT "${CATCH_PATH}" STREQUAL "")
