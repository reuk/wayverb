#include "box/raytracer.h"

#include "raytracer/raytracer.h"

aligned::vector<impulse<8>> run_raytracer(const geo::box& box,
                                          float absorption,
                                          const glm::vec3& source,
                                          const glm::vec3& receiver,
                                          float acoustic_impedance) {
    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const compute_context cc{};

    const auto directions{get_random_directions(1 << 13)};

    auto results{raytracer::run(begin(directions),
                                end(directions),
                                cc,
                                voxelised,
                                source,
                                receiver,
                                true,
                                [](auto i) {})};

    if (! results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    auto impulses{results->get_impulses()};

    for (auto& it : impulses) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }
    return impulses;
}
