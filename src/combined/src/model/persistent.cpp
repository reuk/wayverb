#include "combined/model/persistent.h"

#include "utilities/string_builder.h"

namespace wayverb {
namespace combined {
namespace model {

persistent::persistent(core::geo::box aabb)
        : base_type{sources_t{aabb},
                    receivers_t{aabb},
                    raytracer_t{},
                    waveguide_t{},
                    output_t{},
                    vector<material, 1>{}} {}

shared_value<persistent::sources_t>& persistent::sources() { return get<0>(); }
const shared_value<persistent::sources_t>& persistent::sources() const {
    return get<0>();
}

shared_value<persistent::receivers_t>& persistent::receivers() {
    return get<1>();
}
const shared_value<persistent::receivers_t>& persistent::receivers() const {
    return get<1>();
}

shared_value<persistent::raytracer_t>& persistent::raytracer() {
    return get<2>();
}
const shared_value<persistent::raytracer_t>& persistent::raytracer() const {
    return get<2>();
}

shared_value<persistent::waveguide_t>& persistent::waveguide() {
    return get<3>();
}
const shared_value<persistent::waveguide_t>& persistent::waveguide() const {
    return get<3>();
}

shared_value<persistent::output_t>& persistent::output() { return get<4>(); }
const shared_value<persistent::output_t>& persistent::output() const {
    return get<4>();
}

shared_value<persistent::materials_t>& persistent::materials() {
    return get<5>();
}
const shared_value<persistent::materials_t>& persistent::materials() const {
    return get<5>();
}

////////////////////////////////////////////////////////////////////////////////

std::string compute_output_file_name(const char* unique,
                                     const char* source,
                                     const char* receiver,
                                     const char* capsule) {
    return util::build_string(
            unique, ".s_", source, ".r_", receiver, ".c_", capsule);
}

std::string compute_output_file_name(const char* unique,
                                     const source& source,
                                     const receiver& receiver,
                                     const capsule& capsule) {
    return compute_output_file_name(unique,
                                    source.get_name().c_str(),
                                    receiver.get_name().c_str(),
                                    capsule.get_name().c_str());
}

std::string compute_output_path(const char* directory,
                                const char* unique,
                                const source& source,
                                const receiver& receiver,
                                const capsule& capsule,
                                audio_file::format format) {
    //  TODO platform-dependent, Windows path behaviour is different.
    return util::build_string(
            directory,
            '/',
            compute_output_file_name(unique, source, receiver, capsule),
            '.',
            audio_file::get_extension(format));
}

std::vector<std::string> compute_all_file_names(const char* directory,
                                                const char* unique,
                                                const persistent& persistent) {
    const auto format = persistent.output()->get_format();
    std::vector<std::string> ret;
    for (const auto& source : *persistent.sources()) {
        for (const auto& receiver : *persistent.receivers()) {
            for (const auto& capsule : *receiver->capsules()) {
                ret.emplace_back(compute_output_path(directory,
                                                     unique,
                                                     *source,
                                                     *receiver,
                                                     *capsule,
                                                     format));
            }
        }
    }
    return ret;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
