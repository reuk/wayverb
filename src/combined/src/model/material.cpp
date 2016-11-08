#include "combined/model/material.h"

namespace wayverb {
namespace combined {
namespace model {

void material::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string material::get_name() const { return name_; }

void material::set_surface(core::surface<core::simulation_bands> surface) {
    surface_ = std::move(surface);
    notify();
}

core::surface<core::simulation_bands> material::get_surface() const {
    return surface_;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
