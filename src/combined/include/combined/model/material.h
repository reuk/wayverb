#pragma once

#include "combined/model/vector.h"

#include "core/cl/scene_structs.h"

namespace wayverb {
namespace combined {
namespace model {

class material final : public member<material> {
public:
    material() = default;

    void set_name(std::string name);
    std::string get_name() const;

    void set_surface(core::surface<core::simulation_bands> surface);
    core::surface<core::simulation_bands> get_surface() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(name_, surface_);
        notify();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(name_, surface_);
    }

private:
    std::string name_ = "new material";
    core::surface<core::simulation_bands> surface_;
};

////////////////////////////////////////////////////////////////////////////////

class materials final : public member<materials, vector<material>> {
public:
    materials();

    materials(const materials& other);
    materials(materials&& other) noexcept;

    materials& operator=(const materials& other);
    materials& operator=(materials&& other) noexcept;

    void swap(materials& other) noexcept;

    const material& operator[](size_t index) const;
    material& operator[](size_t index);

    auto cbegin() const { return materials_.cbegin(); }
    auto begin() const { return materials_.begin(); }
    auto begin() { return materials_.begin(); }

    auto cend() const { return materials_.cend(); }
    auto end() const { return materials_.end(); }
    auto end() { return materials_.end(); }

    void insert(size_t index, material t);
    void erase(size_t index);

    size_t size() const;
    bool empty() const;

    void clear();

    template <typename Archive>
    void load(Archive& archive) {
        archive(materials_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(materials_);
    }

private:
    vector<material> materials_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
