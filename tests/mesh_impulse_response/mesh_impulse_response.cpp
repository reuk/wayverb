#include "common/write_audio_file.h"
#include "common/custom_program_base.h"
#include "common/cl_common.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
// std::ostream& operator<<(std::ostream& os, const index& i) {
//    return os << "[" << i.x << ", " << i.y << ", " << i.z << "]";
//}

struct locator {
    size_t x, y, z;
};

constexpr bool operator==(const locator& a, const locator& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr locator to_locator(size_t ind) {
    size_t x = 0;
    for (size_t i = 1;; ++i) {
        size_t prod = i * i;
        if (ind < prod) {
            x = i - 1;
            break;
        }
        ind -= prod;
    }

    size_t y = ind / (x + 1);
    size_t z = ind % (x + 1);

    return locator{x, y, z};
}

static_assert(to_locator(0) == locator{0, 0, 0},  "to_locator");

static_assert(to_locator(1) == locator{1, 0, 0},  "to_locator");
static_assert(to_locator(2) == locator{1, 0, 1},  "to_locator");
static_assert(to_locator(3) == locator{1, 1, 0},  "to_locator");
static_assert(to_locator(4) == locator{1, 1, 1},  "to_locator");

static_assert(to_locator(5) == locator{2, 0, 0},  "to_locator");
static_assert(to_locator(6) == locator{2, 0, 1},  "to_locator");
static_assert(to_locator(7) == locator{2, 0, 2},  "to_locator");
static_assert(to_locator(8) == locator{2, 1, 0},  "to_locator");
static_assert(to_locator(9) == locator{2, 1, 1},  "to_locator");
static_assert(to_locator(10) == locator{2, 1, 2}, "to_locator");
static_assert(to_locator(11) == locator{2, 2, 0}, "to_locator");
static_assert(to_locator(12) == locator{2, 2, 1}, "to_locator");
static_assert(to_locator(13) == locator{2, 2, 2}, "to_locator");

constexpr size_t pyramid(size_t i) {
    return ((2 * i * i * i) + (3 * i * i) + i) / 6;
}

constexpr size_t to_index(const locator& l) {
    return pyramid(l.x) + (l.y * (l.x + 1)) + l.z;
}

static_assert(0  == to_index(locator{0, 0, 0}), "to_index");

static_assert(1  == to_index(locator{1, 0, 0}), "to_index");
static_assert(2  == to_index(locator{1, 0, 1}), "to_index");
static_assert(3  == to_index(locator{1, 1, 0}), "to_index");
static_assert(4  == to_index(locator{1, 1, 1}), "to_index");

static_assert(5  == to_index(locator{2, 0, 0}), "to_index");
static_assert(6  == to_index(locator{2, 0, 1}), "to_index");
static_assert(7  == to_index(locator{2, 0, 2}), "to_index");
static_assert(8  == to_index(locator{2, 1, 0}), "to_index");
static_assert(9  == to_index(locator{2, 1, 1}), "to_index");
static_assert(10 == to_index(locator{2, 1, 2}), "to_index");
static_assert(11 == to_index(locator{2, 2, 0}), "to_index");
static_assert(12 == to_index(locator{2, 2, 1}), "to_index");
static_assert(13 == to_index(locator{2, 2, 2}), "to_index");

class compressed_rectangular_waveguide_program final
        : public custom_program_base {
public:
    explicit compressed_rectangular_waveguide_program(
            const cl::Context& context)
            : custom_program_base(context, source) {
    }

    auto get_kernel() const {
        return custom_program_base::
                get_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(
                        "compressed_waveguide");
    }

private:
    static const std::string source;
};

class compressed_rectangular_waveguide final {
public:
    using kernel_type =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_kernel());
    compressed_rectangular_waveguide(
            const compressed_rectangular_waveguide_program& program,
            cl::CommandQueue& queue,
            size_t dimension)
            : queue(queue)
            , kernel(program.get_kernel())
            , dimension(dimension)
            , storage({{cl::Buffer(
                                program.template get_info<CL_PROGRAM_CONTEXT>(),
                                CL_MEM_READ_WRITE,
                                sizeof(cl_float) * pyramid(dimension + 1)),
                        cl::Buffer(
                                program.template get_info<CL_PROGRAM_CONTEXT>(),
                                CL_MEM_READ_WRITE,
                                sizeof(cl_float) * pyramid(dimension + 1))}})
            , current(&storage[0])
            , previous(&storage[1])
            , output(program.template get_info<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float)) {
    }

    std::vector<float> run() {
        //  init buffers
        std::vector<cl_float> n(pyramid(dimension + 1), 0);
        cl::copy(queue, n.begin(), n.end(), *previous);
        n.front() = 1;
        cl::copy(queue, n.begin(), n.end(), *current);

        std::vector<float> ret;
        for (auto i = 0; i != dimension; ++i) {
            auto o = run_step();
            ret.push_back(o);
            std::cout << o << ", " << std::flush;
        }
        return ret;
    }

private:
    float run_step() {
        kernel(cl::EnqueueArgs(queue, pyramid(dimension)),
               *previous,
               *current,
               output);

        cl_float out;
        cl::copy(queue, output, &out, &out + 1);

        std::swap(previous, current);
        return out;
    }

    cl::CommandQueue& queue;
    kernel_type kernel;
    const size_t dimension;

    std::array<cl::Buffer, 2> storage;
    cl::Buffer* current;
    cl::Buffer* previous;

    cl::Buffer output;
};

const std::string compressed_rectangular_waveguide_program::source {R"(

int3 to_locator(size_t index);
int3 to_locator(size_t index) {
    size_t x = 0;
    for (size_t i = 1;; ++i) {
        size_t prod = i * i;
        if (index < prod) {
            x = i - 1;
            break;
        }
        index -= prod;
    }
    size_t y = index / (x + 1);
    size_t z = index % (x + 1);
    return (int3)(x, y, z);
}

size_t pyramid(size_t i);
size_t pyramid(size_t i) {
    return ((2 * i * i * i) + (3 * i * i) + i) / 6;
}

size_t to_index(int3 l);
size_t to_index(int3 l) {
    return pyramid(l.x) + (l.y * (l.x + 1)) + l.z;
}

void swap(int* a, int* b);
void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int3 fold_locator(int3 i);
int3 fold_locator(int3 i) {
    int x = abs(i.x);
    int y = abs(i.y);
    int z = abs(i.z);
    if (x + 1 <= y) {
        swap(&x, &y);
    }
    if (x + 1 <= z) {
        swap(&x, &z);
    }
    return (int3)(x, y, z);
}

void waveguide_cell_update(global float* prev,
                           const global float* curr,
                           int3 locator);
void waveguide_cell_update(global float* prev,
                           const global float* curr,
                           int3 locator) {
    size_t this_index = to_index(locator);
    prev[this_index] = (curr[to_index(fold_locator(locator + (int3)(-1,  0,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 1,  0,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0, -1,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  1,  0)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  0, -1)))] +
                        curr[to_index(fold_locator(locator + (int3)( 0,  0,  1)))]) / 3.0 - prev[this_index];
}

kernel void compressed_waveguide(global float* previous,
                                 const global float* current,
                                 global float* output) {
    size_t index = get_global_id(0);
    waveguide_cell_update(previous, current, to_locator(index));
    if (index == 0) {
        output[0] = previous[0];
    }
}

)"};

template <typename T>
struct compressed_waveguide final {
    compressed_waveguide(size_t dimension)
            : dimension(dimension) {
        mesh.reserve(dimension);
        for (auto i = 0u; i != dimension; ++i) {
            mesh.push_back(std::vector<std::vector<T>>(
                    i + 1, std::vector<T>(i + 1, 0)));
        }
    }

    T& get(int ix, int iy, int iz) {
        size_t x = std::abs(ix);
        size_t y = std::abs(iy);
        size_t z = std::abs(iz);
        if (x + 1 <= y) {
            std::swap(x, y);
        }
        if (x + 1 <= z) {
            std::swap(x, z);
        }
        assert(y <= x && z <= x);
        return mesh[x][y][z];
    }

    const size_t dimension;

private:
    std::vector<std::vector<std::vector<T>>> mesh;
};

//  given the 'current' and 'previous' mesh states, calculate the 'next' state
//  and write it into the prev array
//
//  assumes courant number = 1 / sqrt(3)
template <typename T>
void waveguide_cell_update(compressed_waveguide<T>* curr,
                           compressed_waveguide<T>* prev,
                           size_t x,
                           size_t y,
                           size_t z) {
    prev->get(x, y, z) = ((curr->get(x - 1, y, z) + curr->get(x + 1, y, z) +
                           curr->get(x, y - 1, z) + curr->get(x, y + 1, z) +
                           curr->get(x, y, z - 1) + curr->get(x, y, z + 1)) /
                          3.0) -
                         prev->get(x, y, z);
}

template <typename T>
void waveguide_mesh_update(compressed_waveguide<T>*& curr,
                           compressed_waveguide<T>*& prev) {
    assert(curr->dimension == prev->dimension);
    //  run waveguide equation for every mesh cell

    auto xlim = curr->dimension - 1;
    for (auto x = 0u; x != xlim; ++x) {
        auto ylim = x + 1;
        for (auto y = 0u; y != ylim; ++y) {
            auto zlim = x + 1;
            for (auto z = 0u; z != zlim; ++z) {
                waveguide_cell_update(curr, prev, x, y, z);
            }
        }
    }

    //  swap meshes
    std::swap(curr, prev);
}

auto run_waveguide(size_t dimension) {
    using waveguide = compressed_waveguide<float>;
    //  set up buffers
    waveguide a{dimension}, b{dimension};
    waveguide* curr = &a;
    waveguide* prev = &b;

    //  init current buffer
    curr->get(0, 0, 0) = 1.0;

    //  set up output
    std::vector<float> output(dimension);

    //  for each output sample
    for (auto& i : output) {
        waveguide_mesh_update(curr, prev);
        i = curr->get(0, 0, 0);
        std::cout << i << ", " << std::flush;
    }
    std::cout << std::endl;

    return output;
}

}  // namespace

int main() {
    ComputeContext c;
    auto program = get_program<compressed_rectangular_waveguide_program>(c);
    compressed_rectangular_waveguide waveguide(program, c.queue, 100);
    auto output = waveguide.run();

    std::cout << std::endl;
    std::cout << std::endl;

    auto verification = run_waveguide(100);
    auto lim = std::min(output.size(), verification.size());
    for (auto i = 0u; i != lim; ++i) {
        assert(verification[i] == output[i]);
    }

    snd::write("mesh_impulse_response.wav", {output}, 44100, 32);
}
