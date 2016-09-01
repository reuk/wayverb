#include "compressed_waveguide.h"
#include "compile_time.h"

#include "common/write_audio_file.h"

#include <iostream>

compressed_rectangular_waveguide::compressed_rectangular_waveguide(
        const compute_context& cc, size_t dimension)
        : queue(cc.context, cc.device)
        , kernel(compressed_rectangular_waveguide_program(cc).get_kernel())
        , dimension(dimension)
        , storage({{cl::Buffer{cc.context,
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * tetrahedron(dimension + 1)},
                    cl::Buffer{cc.context,
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * tetrahedron(dimension + 1)}}})
        , current(&storage[0])
        , previous(&storage[1]) {}

aligned::vector<float> compressed_rectangular_waveguide::run_hard_source(
        aligned::vector<float>&& input) {
    return run(std::move(input),
               &compressed_rectangular_waveguide::run_hard_step);
}

aligned::vector<float> compressed_rectangular_waveguide::run_soft_source(
        aligned::vector<float>&& input) {
    return run(std::move(input),
               &compressed_rectangular_waveguide::run_soft_step);
}

aligned::vector<float> compressed_rectangular_waveguide::run(
        aligned::vector<float>&& input,
        float (compressed_rectangular_waveguide::*step)(float)) {
    //  set input size to maximum valid for this waveguide
    input.resize(dimension * 2);

    //  init buffers
    {
        //  don't want to keep this in scope for too long!
        aligned::vector<cl_float> n(tetrahedron(dimension + 1), 0);
        cl::copy(queue, n.begin(), n.end(), *previous);
        cl::copy(queue, n.begin(), n.end(), *current);
    }

    //  run waveguide for each sample of input
    aligned::vector<float> ret;
    for (auto i : input) {
        //  run the step
        auto o = (this->*step)(i);
        ret.push_back(o);
    }
    return ret;
}

float compressed_rectangular_waveguide::run_hard_step(float i) {
    cl::copy(queue, (&i), (&i) + 1, *current);

    //  run the kernel
    kernel(cl::EnqueueArgs(queue, tetrahedron(dimension)), *previous, *current);

    //  ping-pong the buffers
    std::swap(previous, current);

    //  get output value
    cl_float out;
    cl::copy(queue, *current, &out, &out + 1);

    return out;
}

float compressed_rectangular_waveguide::run_soft_step(float i) {
    //  get current mesh state
    cl_float c;
    cl::copy(queue, *current, (&c), (&c) + 1);

    //  add current input value (soft source)
    c += i;
    cl::copy(queue, (&c), (&c) + 1, *current);

    //  run the kernel
    kernel(cl::EnqueueArgs(queue, tetrahedron(dimension)), *previous, *current);

    //  ping-pong the buffers
    std::swap(previous, current);

    //  get output value
    cl_float out;
    cl::copy(queue, *previous, &out, &out + 1);

    return out;
}

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

)"};

compressed_rectangular_waveguide_program::
        compressed_rectangular_waveguide_program(const compute_context& cc)
        : program_wrapper_(cc, source) {}
