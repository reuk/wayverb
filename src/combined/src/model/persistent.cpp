#include "combined/model/persistent.h"

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

persistent::sources_t& persistent::sources() { return get<0>(); }
const persistent::sources_t& persistent::sources() const { return get<0>(); }

persistent::receivers_t& persistent::receivers() { return get<1>(); }
const persistent::receivers_t& persistent::receivers() const {
    return get<1>();
}

persistent::raytracer_t& persistent::raytracer() { return get<2>(); }
const persistent::raytracer_t& persistent::raytracer() const {
    return get<2>();
}

persistent::waveguide_t& persistent::waveguide() { return get<3>(); }
const persistent::waveguide_t& persistent::waveguide() const {
    return get<3>();
}

persistent::output_t& persistent::output() { return get<4>(); }
const persistent::output_t& persistent::output() const { return get<4>(); }

persistent::materials_t& persistent::materials() { return get<5>(); }
const persistent::materials_t& persistent::materials() const {
    return get<5>();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
