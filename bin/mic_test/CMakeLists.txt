set(name mic_offset_rotate)

add_executable(${name} ${name}.cpp)
target_link_libraries(${name} box waveguide raytracer)
