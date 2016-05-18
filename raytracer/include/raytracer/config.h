#pragma once

#include "common/config.h"

#include <cereal/types/base_class.hpp>

namespace config {

class Raytracer : public virtual App {
public:
    /// Different components of the output impulse.
    enum class OutputMode {
        all,
        image,
        diffuse,
    };

    int rays{1024 * 32};
    int impulses{64};
    float ray_hipass{45};
    bool do_normalize{true};
    bool trim_predelay{false};
    bool trim_tail{false};
    bool remove_direct{false};
    float volume_scale{1.0};
};

}  // namespace config
