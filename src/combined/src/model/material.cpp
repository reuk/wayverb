#include "combined/model/material.h"

namespace wayverb {
namespace combined {
namespace model {

material::material(std::string name,
                   core::surface<core::simulation_bands> surface)
        : name_{std::move(name)}
        , surface_{std::move(surface)} {}

void material::swap(material& other) noexcept {
    using std::swap;
    swap(name_, other.name_);
    swap(surface_, other.surface_);
}

material& material::operator=(material other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

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

bool operator==(const material& a, const material& b) {
    return a.get_name() == b.get_name() && a.get_surface() == b.get_surface();
}

bool operator!=(const material& a, const material& b) { return !(a == b); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
