#pragma once

#include "common/aligned/vector.h"
#include "common/filters_common.h"
#include "common/schroeder.h"
#include "common/stl_wrappers.h"

namespace filter {

class delay_line final {
public:
    delay_line() = default;
    delay_line(int length);

    double& operator[](size_t i);
    const double& operator[](size_t i) const;
    void push(double x);

    void clear();

private:
    aligned::vector<double> data;
    size_t index;
};

//----------------------------------------------------------------------------//

class moving_average final {
public:
    moving_average() = default;
    moving_average(int d);

    double filter(double x);
    void clear();

    double get_index(size_t i) const;

private:
    int d;
    delay_line delay_line;
    double single_delay;
};

//----------------------------------------------------------------------------//

template <int modules>
class n_moving_averages final {
public:
    n_moving_averages() = default;
    n_moving_averages(int d) {
        std::fill(averages.begin(), averages.end(), moving_average(d));
    }

    double filter(double x) {
        for (auto& i : averages) {
            x = i.filter(x);
        }
        return x;
    }

    void clear() {
        for (auto& i : averages) {
            i.clear();
        }
    }

    const moving_average& get_averager() const { return averages.front(); }

private:
    std::array<moving_average, modules> averages;
};

//----------------------------------------------------------------------------//

class linear_dc_blocker final {
public:
    explicit linear_dc_blocker(int d = 128);

    double filter(double x);

    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        aligned::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->filter(i);
        });
        return ret;
    }

    void clear();

private:
    int d;
    n_moving_averages<2> moving_averages;
};

//----------------------------------------------------------------------------//

class extra_linear_dc_blocker final {
public:
    explicit extra_linear_dc_blocker(int d = 128);

    double filter(double x);

    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        aligned::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->filter(i);
        });
        return ret;
    }

    void clear();

private:
    int d;
    delay_line delay_line;
    n_moving_averages<4> moving_averages;
};

//----------------------------------------------------------------------------//

/// This uses a butterworth filter for a flat passband and steep falloff.
/// The butterworth is chosen to provide a reasonable balance between stope
/// steepness and impulse response length.
/// To maintain zero-phase the filter is run forwards then backwards over the
/// input, doubling the steepness of the falloff.
/// It will also introduce some pre- and post-ring.
template <typename It>
void block_dc(It begin, It end, double sr) {
    auto hipass{make_series_biquads(
            compute_hipass_butterworth_coefficients<1>(10, sr))};
    run_two_pass(hipass, begin, end);
}

}  // namespace filter
