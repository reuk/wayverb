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
                                              vector<material, 1>> {
public:
    using sources_t = sources;
    using receivers_t = receivers;
    using raytracer_t = class raytracer;
    using waveguide_t = class waveguide;
    using materials_t = class vector<material, 1>;

    shared_value<sources_t>& sources();
    const shared_value<sources_t>& sources() const;

    shared_value<receivers_t>& receivers();
    const shared_value<receivers_t>& receivers() const;

    shared_value<raytracer_t>& raytracer();
    const shared_value<raytracer_t>& raytracer() const;

    shared_value<waveguide_t>& waveguide();
    const shared_value<waveguide_t>& waveguide() const;

    shared_value<materials_t>& materials();
    const shared_value<materials_t>& materials() const;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
