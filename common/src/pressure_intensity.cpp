#include "common/pressure_intensity.h"
#include "common/interpolation.h"

/*
aligned::vector<float> energy_histogram_to_impulse_response(
        const aligned::vector<volume_type>& histogram,
        double period,
        double sample_rate) {
    for (const auto & i : histogram) {
        //  find smoothed energy spectrum
        const auto energy_spectrum{smooth_energy_bands(i, sample_rate)};

        //  find appropriate minimum phase spectrum
    }
}
*/
