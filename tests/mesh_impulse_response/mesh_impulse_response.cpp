#include "common/write_audio_file.h"

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
    auto output = run_waveguide(1000);
    snd::write("mesh_impulse_response.wav", {output}, 44100, 32);
}
