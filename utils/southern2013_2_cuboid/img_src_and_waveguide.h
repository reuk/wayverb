#pragma once

#include "image_source.h"
#include "waveguide.h"

class img_src_and_waveguide_test final {
public:
    img_src_and_waveguide_test(const generic_scene_data<cl_float3, surface>& sd,
                               float speed_of_sound,
                               float acoustic_impedance);

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::receiver_settings& receiver);

private:
    compute_context compute_context_{};
    double waveguide_sample_rate_{18000.0};
    std::tuple<voxelised_scene_data<cl_float3, surface>, waveguide::mesh>
            voxels_and_mesh_;
    float speed_of_sound_;
    float acoustic_impedance_;
};
