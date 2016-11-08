#pragma once

#include "combined/model/member.h"

#include "raytracer/simulation_parameters.h"

namespace wayverb {
namespace combined {
namespace model {

class raytracer final : public member<raytracer> {
public:
    raytracer() = default;

    raytracer(const raytracer&) = default;
    raytracer(raytracer&&) noexcept = default;

    raytracer& operator=(const raytracer&) = default;
    raytracer& operator=(raytracer&&) noexcept = default;

    void set_rays(size_t rays);
    void set_max_img_src_order(size_t max);

    wayverb::raytracer::simulation_parameters get() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(data_.rays, data_.maximum_image_source_order);
        notify();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_.rays, data_.maximum_image_source_order);
    }

private:
    wayverb::raytracer::simulation_parameters data_{10000, 4};
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
