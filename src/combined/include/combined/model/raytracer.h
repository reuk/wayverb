#pragma once

#include "combined/model/member.h"

#include "raytracer/simulation_parameters.h"

namespace wayverb {
namespace combined {
namespace model {

class raytracer final : public member<raytracer> {
public:
    raytracer() = default;

    raytracer(const raytracer&) = delete;
    raytracer(raytracer&&) noexcept = delete;

    raytracer& operator=(const raytracer&) = delete;
    raytracer& operator=(raytracer&&) noexcept = delete;

    void set_rays(size_t rays);
    void set_max_img_src_order(size_t max);

    wayverb::raytracer::simulation_parameters get() const;

private:
    wayverb::raytracer::simulation_parameters data_{10000, 4};
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
