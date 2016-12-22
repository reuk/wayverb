set(name solution_growth)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide)
