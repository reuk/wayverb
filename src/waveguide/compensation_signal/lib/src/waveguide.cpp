#include "compensation_signal/waveguide.h"

#include <iostream>

compressed_rectangular_waveguide::compressed_rectangular_waveguide(
        const compute_context& cc, size_t steps)
        : queue_{cc.context, cc.device}
        , compressed_waveguide_kernel_{compressed_rectangular_waveguide_program{
                  cc}.get_compressed_waveguide_kernel()}
        , zero_buffer_kernel_{compressed_rectangular_waveguide_program{
                  cc}.get_zero_buffer_kernel()}
        , dimension_{(steps + 1) / 2}
        , current_{cc.context,
                   CL_MEM_READ_WRITE,
                   sizeof(cl_float) * tetrahedron(dimension_ + 1)}
        , previous_{cc.context,
                    CL_MEM_READ_WRITE,
                    sizeof(cl_float) * tetrahedron(dimension_ + 1)} {}

constexpr const char* source{R"(
int triangle(int i);
int triangle(int i) {
    return (i * (i + 1)) / 2;
}

int tetrahedron(int i);
int tetrahedron(int i) {
    return (i * (i + 1) * (i + 2)) / 6;
}

void swap(int* a, int* b);
void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int3 to_locator(int ind);
int3 to_locator(int ind) {
    int x = 0;
    for (int i = 1;; ++i) {
        int tri = triangle(i);
        if (ind < tri) {
            x = i - 1;
            break;
        }
        ind -= tri;
    }

    int y = 0;
    for (int i = 1;; ++i) {
        if (ind < i) {
            y = i - 1;
            break;
        }
        ind -= i;
    }

    int z = ind;

    return (int3)(x, y, z);
}

int3 fold_locator(int3 i);
int3 fold_locator(int3 i) {
    int x = abs(i.x);
    int y = abs(i.y);
    int z = abs(i.z);
    int plane = x + 1;
    if (plane <= y) {
        swap(&x, &y);
    }
    if (plane <= z) {
        swap(&x, &z);
    }
    if (y < z) {
        swap(&y, &z);
    }
    return (int3)(x, y, z);
}

int to_index(int3 l);
int to_index(int3 l) {
    return tetrahedron(l.x) + triangle(l.y) + l.z;
}

void waveguide_cell_update(global float* prev,
                           const global float* curr,
                           int3 locator);
void waveguide_cell_update(global float* prev,
                           const global float* curr,
                           int3 locator) {
    int this_index = to_index(locator);
    prev[this_index] = (curr[to_index(fold_locator(locator + (int3)(-1,  0,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 1,  0,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0, -1,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  1,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  0, -1)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  0,  1)))]) / 3.0 - prev[this_index];
}

kernel void compressed_waveguide(global float* previous,
                                 const global float* current) {
    int index = get_global_id(0);
    waveguide_cell_update(previous, current, to_locator(index));
}

kernel void zero_buffer(global float* buf) {
    size_t thread = get_global_id(0);
    buf[thread] = 0;
}

)"};

compressed_rectangular_waveguide_program::
        compressed_rectangular_waveguide_program(const compute_context& cc)
        : program_wrapper_(cc, source) {}
