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
                                  sources,
                                  receivers,
                                  raytracer,
                                  waveguide,
                                  output> {
public:
    scene(core::geo::box aabb);

    scene(const scene& other);
    scene(scene&& other) noexcept;

    scene& operator=(const scene& other);
    scene& operator=(scene&& other) noexcept;

    void swap(scene& other) noexcept;

    template <typename Archive>
    void load(Archive& archive) {
        archive(aabb_, sources, receivers, raytracer, waveguide, output);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(aabb_, sources, receivers, raytracer, waveguide, output);
    }

private:
    core::geo::box aabb_;

public:
    sources sources;
    receivers receivers;
    raytracer raytracer;
    waveguide waveguide;
    output output;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
