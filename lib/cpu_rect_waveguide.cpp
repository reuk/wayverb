#include "cpu_rect_waveguide.h"

CpuRectangularWaveguide::CpuRectangularWaveguide(const Boundary& boundary,
                                                 float spacing,
                                                 const Vec3f& anchor)
        : mesh(boundary, spacing, anchor)
        , storage({{Collection(mesh.get_nodes().size()),
                    Collection(mesh.get_nodes().size())}})
        , previous(&storage[0])
        , current(&storage[1]) {
}

const RectangularMesh& CpuRectangularWaveguide::get_mesh() const {
    return mesh;
}

void CpuRectangularWaveguide::swap_buffers() {
    std::swap(previous, current);
}

void CpuRectangularWaveguide::initialise_mesh(const PowerFunction& u,
                                              const Vec3f& excitation,
                                              std::vector<float>& ret) {
    const auto& nodes = mesh.get_nodes();
    auto size = nodes.size();
    ret.resize(size);
    for (auto i = 0u; i != size; ++i) {
        if (nodes[i].inside)
            ret[i] = u(to_vec3f(nodes[i].position), excitation);
    }
}

void CpuRectangularWaveguide::init(const Vec3f& e,
                                   const PowerFunction& u,
                                   size_type o,
                                   float sr) {
    setup(o, sr);
    std::fill(previous->begin(), previous->end(), 0);
    std::fill(current->begin(), current->end(), 0);
    initialise_mesh(u, e, *current);
}

void waveguide_kernel(int index,
                      const std::vector<float>& current,
                      std::vector<float>& previous,
                      std::vector<float>& next,
                      const std::vector<CpuRectangularWaveguide::Node>& nodes,
                      const std::vector<float>& transform_matrix,
                      Vec3f& velocity_buffer,
                      float spatial_sampling_period,
                      float T,
                      int read,
                      float& output) {
    const auto& node = nodes[index];

    if (node.inside) {
        return;
    }

    auto temp = 0.0f;

    for (auto i = 0u; i != CpuRectangularWaveguide::PORTS; ++i) {
        auto port_index = node.ports[i];
        if (port_index >= 0 && nodes[port_index].inside) {
            temp += current[port_index];
        }
    }

    temp /= 3;
    temp -= previous[index];

    next[index] = temp;

    if (index == read) {
        output = temp;

        auto differences =
            std::array<float, CpuRectangularWaveguide::PORTS>{{0}};
        for (auto i = 0u; i != CpuRectangularWaveguide::PORTS; ++i) {
            int port_index = node.ports[i];
            if (port_index >= 0 && nodes[port_index].inside)
                differences[i] = (previous[port_index] - previous[index]) /
                                 spatial_sampling_period;
        }

        auto multiplied = Vec3f(0);
        for (auto i = 0u; i != CpuRectangularWaveguide::PORTS; ++i) {
            multiplied = multiplied +
                         Vec3f(transform_matrix.data()[0 + i * 3],
                               transform_matrix.data()[1 + i * 3],
                               transform_matrix.data()[2 + i * 3]) *
                             differences[i];
        }

        float ambient_density = 1.225;
        multiplied = multiplied / -ambient_density;

        velocity_buffer = velocity_buffer + multiplied * T;
    }
}

void CpuRectangularWaveguide::setup(size_type o, float sr) {
    transform_matrix = get_transform_matrix(
        CpuRectangularWaveguide::PORTS, o, mesh.get_nodes());
    velocity_buffer = Vec3f(0);
    period = 1 / sr;
}

std::vector<RunStepResult> CpuRectangularWaveguide::run(const Vec3f& e,
                                                        const PowerFunction& u,
                                                        size_type o,
                                                        size_type steps,
                                                        float sr) {
    init(e, u, o, sr);

    std::vector<RunStepResult> ret(steps);
    auto counter = 0u;
    std::generate(ret.begin(),
                  ret.end(),
                  [this, &counter, &steps, &o] {
                      auto ret = this->run_step(o, *previous, *current);
                      this->swap_buffers();

                      auto percent = counter * 100 / (steps - 1);
                      std::cout << "\r" << percent << "% done" << std::flush;

                      counter += 1;

                      return ret;
                  });

    std::cout << std::endl;

    return ret;
}

RunStepResult CpuRectangularWaveguide::run_step(size_type o,
                                                Collection& previous,
                                                const Collection& current) {
    std::vector<float> next(mesh.get_nodes().size());
    float output;

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        waveguide_kernel(
            i,
            current,
            previous,
            next,
            mesh.get_nodes(),
            std::vector<float>(
                transform_matrix.data(),
                transform_matrix.data() + CpuRectangularWaveguide::PORTS * 3),
            velocity_buffer,
            mesh.get_spacing(),
            period,
            o,
            output);
    }

    previous = std::move(next);

    auto intensity = velocity_buffer * output;
    return RunStepResult(output, intensity);
}

CpuRectangularWaveguide::Collection CpuRectangularWaveguide::run_step_slow() {
    run_step(0, *previous, *current);
    swap_buffers();
    return *previous;
}
