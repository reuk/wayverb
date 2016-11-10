#pragma once

#include "combined/model/vector.h"

#include "core/cl/scene_structs.h"

namespace wayverb {
namespace combined {
namespace model {

class material final : public basic_member<material> {
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

template <size_t MinimumSize, typename It>
vector<material, MinimumSize> materials_from_names(It b, It e) {
    struct material_from_name final {
        auto operator()(const std::string& str) const { return material{str}; }
    };

    const auto make_iterator = [&](auto it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   material_from_name{});
    };

    return vector<material, MinimumSize>{make_iterator(std::move(b)),
                                         make_iterator(std::move(e))};
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
