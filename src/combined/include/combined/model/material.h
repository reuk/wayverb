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
                              core::make_surface<core::simulation_bands>(0.05,
                                                                         0.05));

    void set_name(std::string name);
    std::string get_name() const;

    void set_surface(core::surface<core::simulation_bands> surface);
    core::surface<core::simulation_bands> get_surface() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(name_, surface_);
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
    const auto extra = std::distance(b, e) - MinimumSize;

    vector<material, MinimumSize> ret{extra};

    for (auto i = 0; b != e; ++b, ++i) {
        *ret[i] = material{*b};
    }

    return ret;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
