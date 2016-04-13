#pragma once

#include "vec.h"

#define SPEED_OF_SOUND (340.0f)

namespace config {

class App {
public:
    virtual ~App() noexcept = default;

    Vec3f& get_source();
    Vec3f& get_mic();
    float& get_output_sample_rate();
    int& get_bit_depth();

    Vec3f get_source() const;
    Vec3f get_mic() const;
    float get_output_sample_rate() const;
    int get_bit_depth() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(source, mic, sample_rate, bit_depth);
    }

private:
    Vec3f source{0, 0, 0};
    Vec3f mic{0, 0, 0};
    float sample_rate{44100};
    int bit_depth{16};
};
}  // namespace config
