#include "hrtf_attenuator.h"
#include "filters_common.h"
#include "hrtf.h"
#include "stl_wrappers.h"

HrtfAttenuator::HrtfAttenuator(const Vec3f& direction,
                               const Vec3f& up,
                               int channel,
                               float sr)
        : direction(direction)
        , up(up)
        , channel(channel)
        , sr(sr) {
}

Vec3f transform(const Vec3f& pointing, const Vec3f& up, const Vec3f& d) {
    auto x = up.cross(pointing).normalized();
    auto y = pointing.cross(x);
    auto z = pointing;
    return Vec3f(x.dot(d), y.dot(d), z.dot(d));
}

float azimuth(const Vec3f& d) {
    return atan2(d.x, d.z);
}

float elevation(const Vec3f& d) {
    return atan2(d.y, Vec3f(d.x, d.z, 0).mag());
}

float degrees(float radians) {
    return radians * 180 / M_PI;
}

float HrtfAttenuator::attenuation(const Vec3f& incident, int band) const {
    auto transformed = transform(direction, up, incident);
    int a = degrees(azimuth(transformed)) + 180;
    a %= 360;
    int e = degrees(elevation(transformed));
    e = 90 - e;
    return HrtfData::HRTF_DATA[channel][a][e].s[band];
}

Vec3f HrtfAttenuator::get_direction() const {
    return direction;
}

Vec3f HrtfAttenuator::get_up() const {
    return up;
}

std::vector<float> HrtfAttenuator::process(
    const std::vector<RunStepResult>& input) const {
    std::vector<float> ret(input.size(), 0);
    filter::LinkwitzRileyBandpass bandpass;
    for (auto band = 0u; band != sizeof(VolumeType) / sizeof(float); ++band) {
        if (HrtfData::EDGES[band + 1] < sr / 2) {
            std::vector<float> this_band(input.size());
            proc::transform(input, this_band.begin(), [this, band](auto i) {
                //  TODO DEFINITELY CHECK THIS
                //  RUN TESTS YEAH
                auto mag = i.intensity.mag();
                if (mag == 0)
                    return 0.0f;
                mag = sqrt(mag * pow(attenuation(i.intensity, band), 2));
                return std::copysign(mag, i.pressure);
            });

            bandpass.set_params(
                HrtfData::EDGES[band], HrtfData::EDGES[band + 1], sr);
            bandpass.filter(this_band);

            proc::transform(
                this_band, ret.begin(), ret.begin(), [](auto a, auto b) {
                    return a + b;
                });
        }
    }

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}
