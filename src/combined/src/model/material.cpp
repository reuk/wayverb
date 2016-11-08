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

////////////////////////////////////////////////////////////////////////////////

materials::materials() { connect(materials_); }

void materials::swap(materials& other) noexcept {
    using std::swap;
    swap(materials_, other.materials_);
}

materials::materials(const materials& other)
        : materials_{other.materials_} {
    connect(materials_);
}

materials::materials(materials&& other) noexcept {
    swap(other);
    connect(materials_);
}

materials& materials::operator=(const materials& other) {
    auto copy{other};
    swap(copy);
    connect(materials_);
    return *this;
}

materials& materials::operator=(materials&& other) noexcept {
    swap(other);
    connect(materials_);
    return *this;
}

const material& materials::operator[](size_t index) const {
    return materials_[index];
}
material& materials::operator[](size_t index) { return materials_[index]; }

void materials::insert(size_t index, material t) {
    materials_.insert(materials_.begin() + index, std::move(t));
}

void materials::erase(size_t index) {
    if (1 < materials_.size()) {
        materials_.erase(materials_.begin() + index);
    }
}

size_t materials::size() const { return materials_.size(); }

bool materials::empty() const { return materials_.empty(); }

void materials::clear() { materials_.clear(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
