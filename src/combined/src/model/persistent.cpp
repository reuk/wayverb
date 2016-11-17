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

}  // namespace model
}  // namespace combined
}  // namespace wayverb
