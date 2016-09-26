#pragma once

#include "audio.h"

#include "common/cl/common.h"
#include "common/model/receiver_settings.h"
#include "common/spatial_division/voxelised_scene_data.h"

class image_source_test final {
public:
    image_source_test(const generic_scene_data<cl_float3, surface>& sd,
                      float speed_of_sound,
                      float acoustic_impedance);

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::receiver_settings& receiver);

private:
    compute_context compute_context_{};
    voxelised_scene_data<cl_float3, surface> voxelised_;
    float speed_of_sound_;
    float acoustic_impedance_;
};

