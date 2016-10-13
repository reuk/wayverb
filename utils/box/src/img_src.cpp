#include "box/img_src.h"

#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

aligned::vector<impulse<8>> run_exact_img_src(const geo::box& box,
                                              float absorption,
                                              const glm::vec3& source,
                                              const glm::vec3& receiver,
                                              float speed_of_sound,
                                              float acoustic_impedance,
                                              float simulation_time,
                                              bool flip_phase) {
    auto ret{raytracer::image_source::find_impulses<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            box,
            source,
            receiver,
            make_surface(absorption, 0),
            simulation_time * speed_of_sound,
            flip_phase)};
    //  Correct for distance travelled.
    for (auto& it : ret) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }
    return ret;
}

aligned::vector<impulse<8>> run_fast_img_src(const geo::box& box,
                                             float absorption,
                                             const glm::vec3& source,
                                             const glm::vec3& receiver,
                                             float acoustic_impedance,
                                             bool flip_phase) {
    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const auto directions{get_random_directions(1 << 13)};
    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            begin(directions),
            end(directions),
            compute_context{},
            voxelised,
            source,
            receiver,
            flip_phase)};

    if (const auto direct{raytracer::get_direct(source, receiver, voxelised)}) {
        impulses.emplace_back(*direct);
    }
    //  Correct for distance travelled.
    for (auto& it : impulses) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }
    return impulses;
}
