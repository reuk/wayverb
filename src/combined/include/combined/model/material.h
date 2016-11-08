#pragma once

#include "combined/model/member.h"

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

private:
    std::string name_ = "new material";
    core::surface<core::simulation_bands> surface_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
