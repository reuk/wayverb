#pragma once

#include "audio.h"

#include "waveguide/mesh.h"

#include "common/model/receiver_settings.h"

class waveguide_test final {
public:
    waveguide_test(const scene_data& sd,
                   float speed_of_sound,
                   float acoustic_impedance);

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::receiver_settings& receiver);

private:
    compute_context compute_context_{};
    double sample_rate_{10000.0};
    std::tuple<voxelised_scene_data, waveguide::mesh> voxels_and_mesh_;
    float speed_of_sound_;
    float acoustic_impedance_;
};
