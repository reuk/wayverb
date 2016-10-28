#include "waveguide/fitted_boundary.h"
#include "waveguide/serialize/coefficients_canonical.h"

#include "utilities/named_value.h"
#include "utilities/string_builder.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include <iostream>

struct reflectance_and_impedance final {
    coefficients_canonical reflectance;
    coefficients_canonical impedance;
};

namespace cereal {

template <typename Archive, typename T>
void serialize(Archive& archive, named_value<T>& named) {
    archive(make_nvp(named.name, named.value));
}

template <typename Archive>
void serialize(Archive& archive, reflectance_and_impedance& rai) {
    archive(make_nvp("reflectance", rai.reflectance),
            make_nvp("impedance", rai.impedance));
}

}  // namespace cereal

template <size_t... Ix>
constexpr auto make_slope_array(double begin,
                                double end,
                                std::index_sequence<Ix...>) {
    return std::array<double, sizeof...(Ix)>{
            {linear_interp(Ix / (sizeof...(Ix) - 1.0), begin, end)...}};
}

template <size_t N>
constexpr auto make_slope_array(double begin, double end) {
    return make_slope_array(begin, end, std::make_index_sequence<N>{});
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    constexpr auto sample_rate = 44100.0;

    const auto make_reflectance_coefficients = [&](const auto& absorption) {
        return waveguide::compute_reflectance_filter_coefficients(absorption,
                                                                  sample_rate);
    };

    const auto reflectance = std::vector<named_value<coefficients_canonical>>{
            {"sloping_fitted_0",
             make_reflectance_coefficients(
                     make_slope_array<simulation_bands>(0, 1))},

            {"sloping_fitted_1",
             make_reflectance_coefficients(
                     make_slope_array<simulation_bands>(1, 0))},

            {"sudden",
             make_reflectance_coefficients(std::array<double, simulation_bands>{
                     {0, 1, 0, 1, 0, 1, 0, 1}})},
    };

    const auto output = map_to_vector(
            begin(reflectance), end(reflectance), [](const auto& i) {
                return map(
                        [](const auto& i) {
                            return reflectance_and_impedance{
                                    i, waveguide::to_impedance_coefficients(i)};
                        },
                        i);
            });

    cereal::JSONOutputArchive{std::cout}(output);
}
