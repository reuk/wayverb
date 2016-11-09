#pragma once

#include "combined/model/vector.h"

#include "core/cl/scene_structs.h"

namespace wayverb {
namespace combined {
namespace model {

class material final : public member<material> {
public:
    explicit material(std::string name = "new material",
                      core::surface<core::simulation_bands> surface =
                              core::surface<core::simulation_bands>());

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
    std::string name_;
    core::surface<core::simulation_bands> surface_;
};

////////////////////////////////////////////////////////////////////////////////

namespace {

struct make_material_from_string final {
    auto operator()(const std::string& str) const { return material{str}; }
};

}  // namespace

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

    template <typename It>
    void set_from_strings(It b, It e) {
        if (std::distance(b, e) <= 1) {
            throw std::runtime_error{"must init with at least 1 material"};
        }

        const auto make_iterator = [&](auto it) {
            return util::make_mapping_iterator_adapter(
                    std::move(it), make_material_from_string{});
        };

        materials_ = vector<material>(make_iterator(std::move(b)),
                                      make_iterator(std::move(e)));
    }

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
