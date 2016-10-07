/// A program to read a bunch of HRTF kernels and create a simple c++ source
/// file containing an array of hrtf::entry structs.
/// These can be processed at run-time to produce a fast lookup table (or a
/// slow lookup table or whatever).

#include "hrtf/entry.h"

#include "dir.h"

#include "frequency_domain/multiband_filter.h"

#include "audio_file/audio_file.h"

#include <array>
#include <experimental/optional>
#include <numeric>
#include <regex>

int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error{"expected a directory name"};
    }

    constexpr auto bands{8};
    constexpr range<double> audible{20, 20000};

    std::array<std::vector<hrtf::entry<bands>>, 2> results;

    const auto entries{list_directory(argv[1])};
    const std::regex name_regex{".*R([0-9]+}_T([0-9]+)_P([0-9]+).*"};
    for (const auto& entry : entries) {
        std::smatch match{};
        if (std::regex_match(entry, match, name_regex)) {
            const auto az{std::stoi(match[2].str())};
            const auto el{std::stoi(match[3].str())};

            const auto audio{audio_file::read(entry)};

            if (audio.signal.size() != 2) {
                throw std::runtime_error{"hrtf data files must be stereo"};
            }

            const range<double> normalised_range{
                    audible.get_min() / audio.sample_rate,
                    audible.get_max() / audio.sample_rate};

            for (auto i{0ul}; i != 2; ++i) {
                results[i].emplace_back(hrtf::entry<bands>{
                        az,
                        el,
                        frequency_domain::per_band_energy<bands>(
                                begin(audio.signal[i]),
                                end(audio.signal[i]),
                                normalised_range)});
            }
        }
    }

    return EXIT_SUCCESS;
}
