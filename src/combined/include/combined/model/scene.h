#pragma once

#include "combined/model/material.h"
#include "combined/model/output.h"
#include "combined/model/raytracer.h"
#include "combined/model/receiver.h"
#include "combined/model/source.h"
#include "combined/model/waveguide.h"

namespace wayverb {
namespace combined {
namespace model {

class scene final : public member<scene,
                                  raytracer,
                                  waveguide,
                                  output,
                                  vector<source>,
                                  vector<receiver>> {
public:
    scene(core::geo::box aabb);

    const class source& source(size_t index) const;
    class source& source(size_t index);
    void add_source(size_t index);
    void remove_source(size_t index);

    const class receiver& receiver(size_t index) const;
    class receiver& receiver(size_t index);
    void add_receiver(size_t index);
    void remove_receiver(size_t index);

    raytracer raytracer;
    waveguide waveguide;
    output output;

private:
    core::geo::box aabb_;

    vector<class source> sources_;
    vector<class receiver> receivers_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
