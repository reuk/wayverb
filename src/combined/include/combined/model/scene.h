#pragma once

#include "combined/model/material.h"
#include "combined/model/output.h"
#include "combined/model/raytracer.h"
#include "combined/model/receiver.h"
#include "combined/model/source.h"
#include "combined/model/waveguide.h"

namespace wayverb {
namespace combined {
namespace model {

class scene final : public owning_member<scene,
                                         sources,
                                         receivers,
                                         raytracer,
                                         waveguide,
                                         output> {
public:
    explicit scene(core::geo::box aabb);

    using sources_t = class sources;
    using receivers_t = class receivers;
    using raytracer_t = class raytracer;
    using waveguide_t = class waveguide;
    using output_t = class output;

    sources_t& sources();
    const sources_t& sources() const;

    receivers_t& receivers();
    const receivers_t& receivers() const;

    raytracer_t& raytracer();
    const raytracer_t& raytracer() const;

    waveguide_t& waveguide();
    const waveguide_t& waveguide() const;

    output_t& output();
    const output_t& output() const;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
