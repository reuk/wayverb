#pragma once

#include "audio.h"

#include "common/model/receiver_settings.h"
#include "common/spatial_division/voxelised_scene_data.h"

class image_source_test final {
public:
    image_source_test(const scene_data& sd,
                      float speed_of_sound,
                      float acoustic_impedance);

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::receiver_settings& receiver);

private:
    compute_context compute_context_{};
    voxelised_scene_data voxelised_;
    float speed_of_sound_;
    float acoustic_impedance_;
};

