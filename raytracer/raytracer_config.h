#pragma once

#include "app_config.h"

#include <cereal/types/base_class.hpp>

namespace config {

class Raytracer : public virtual App {
public:
    virtual ~Raytracer() noexcept = default;

    /// Different components of the output impulse.
    enum class OutputMode {
        all,
        image,
        diffuse,
    };

    int& get_rays();
    int& get_impulses();
    float& get_ray_hipass();
    bool& get_do_normalize();
    bool& get_trim_predelay();
    bool& get_trim_tail();
    bool& get_remove_direct();
    float& get_volume_scale();

    int get_rays() const;
    int get_impulses() const;
    float get_ray_hipass() const;
    bool get_do_normalize() const;
    bool get_trim_predelay() const;
    bool get_trim_tail() const;
    bool get_remove_direct() const;
    float get_volume_scale() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::virtual_base_class<App>(this),
                rays,
                impulses,
                ray_hipass,
                do_normalize,
                trim_predelay,
                trim_tail,
                remove_direct,
                volume_scale);
    }

private:
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
