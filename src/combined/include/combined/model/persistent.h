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

/// Holds basically all the app state that will get written to file,
/// other than presets, which will be stored separately.

class persistent final : public owning_member<persistent,
                                              sources,
                                              receivers,
                                              raytracer,
                                              waveguide,
                                              output,
                                              vector<material, 1>> {
public:
    persistent();
    explicit persistent(core::geo::box aabb);

    using sources_t = class sources;
    using receivers_t = class receivers;
    using raytracer_t = class raytracer;
    using waveguide_t = class waveguide;
    using output_t = class output;
    using materials_t = class vector<material, 1>;

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

    materials_t& materials();
    const materials_t& materials() const;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
