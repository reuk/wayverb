#pragma once

#include "combined/model/member.h"

#include "raytracer/simulation_parameters.h"

namespace wayverb {
namespace combined {
namespace model {

class raytracer final : public basic_member<raytracer> {
public:
    explicit raytracer(double room_volume = 0,
                       double speed_of_sound = 340,
                       size_t quality = 3,
                       size_t img_src_order = 4,
                       double receiver_radius = 0.1,
                       double histogram_sample_rate = 1000);

    void set_quality(size_t quality);
    size_t get_quality() const;

    void set_max_img_src_order(size_t max);
    size_t get_max_img_src_order() const;

    void set_room_volume(double volume);

    wayverb::raytracer::simulation_parameters get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(quality_, img_src_order_);
    }

    NOTIFYING_COPY_ASSIGN_DECLARATION(raytracer)
private:
    void swap(raytracer& other) noexcept {
        using std::swap;
        swap(quality_, other.quality_);
        swap(img_src_order_, other.img_src_order_);
    }

    double room_volume_;
    double speed_of_sound_;

    size_t quality_;
    size_t img_src_order_;

    double receiver_radius_;
    double histogram_sample_rate_;
};

bool operator==(const raytracer& a, const raytracer& b);
bool operator!=(const raytracer& a, const raytracer& b);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
