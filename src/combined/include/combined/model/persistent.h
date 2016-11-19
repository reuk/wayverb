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
    explicit persistent(core::geo::box aabb);

    using sources_t = class sources;
    using receivers_t = class receivers;
    using raytracer_t = class raytracer;
    using waveguide_t = class waveguide;
    using output_t = class output;
    using materials_t = class vector<material, 1>;

    shared_value<sources_t>& sources();
    const shared_value<sources_t>& sources() const;

    shared_value<receivers_t>& receivers();
    const shared_value<receivers_t>& receivers() const;

    shared_value<raytracer_t>& raytracer();
    const shared_value<raytracer_t>& raytracer() const;

    shared_value<waveguide_t>& waveguide();
    const shared_value<waveguide_t>& waveguide() const;

    shared_value<output_t>& output();
    const shared_value<output_t>& output() const;

    shared_value<materials_t>& materials();
    const shared_value<materials_t>& materials() const;
};

////////////////////////////////////////////////////////////////////////////////

std::string compute_output_file_name(const char* unique,
                                     const char* source,
                                     const char* receiver,
                                     const char* capsule);

std::string compute_output_file_name(const char* unique,
                                     const source& source,
                                     const receiver& receiver,
                                     const capsule& capsule);

std::string compute_output_path(const char* directory,
                                const char* unique,
                                const source& source,
                                const receiver& receiver,
                                const capsule& capsule,
                                audio_file::format format);

std::vector<std::string> compute_all_file_names(const char* directory,
                                                const char* unique,
                                                const persistent& persistent);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
