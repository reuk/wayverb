#pragma once

#include "combined/model/min_size_vector.h"

#include "core/cl/scene_structs.h"
#include "core/serialize/surface.h"

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
    void serialize(Archive& archive) {
        archive(name_, surface_);
    }

    NOTIFYING_COPY_ASSIGN_DECLARATION(material)
private:
    void swap(material& other) noexcept {
        using std::swap;
        swap(name_, other.name_);
        swap(surface_, other.surface_);
    }

    std::string name_;
    core::surface<core::simulation_bands> surface_;
};

bool operator==(const material& a, const material& b);
bool operator!=(const material& a, const material& b);

////////////////////////////////////////////////////////////////////////////////

template <size_t MinimumSize, typename It>
auto materials_from_names(It b, It e) {
    const auto distance = std::distance(b, e);
    if (distance < MinimumSize) {
        throw std::runtime_error{
                "Range passed to 'materials_from_names' is shorter than the "
                "minumum size"};
    }
    const auto extra = distance - MinimumSize;

    min_size_vector<material, MinimumSize> ret{extra};

    for (auto i = 0; b != e; ++b, ++i) {
        ret[i]->item = material{*b};
    }

    return ret;
}
}  // namespace model
}  // namespace combined
}  // namespace wayverb
