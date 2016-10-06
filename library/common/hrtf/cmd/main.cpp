/// A program to read a bunch of HRTF kernels and create a simple c++ source
/// file containing an array of hrtf::entry structs.
/// These can be processed at run-time to produce a fast lookup table (or a
/// slow lookup table or whatever).

#include "hrtf/entry.h"

#include "dir.h"

#include "common/per_band_energy.h"

#include <array>
#include <numeric>
#include <regex>

int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::runtime_error{"expected a directory name"};
    }

    constexpr auto bands{8};

    std::vector<hrtf::entry<bands>> results;

    const auto entries{list_directory(argv[1])};
    const std::regex name_regex{".*R([0-9]+}_T([0-9]+)_P([0-9]+).*"};
    for (const auto& entry : entries) {
        std::smatch match{};
        if (std::regex_match(entry, match, name_regex)) {
            const auto az{std::stoi(match[2].str())};
            const auto el{std::stoi(match[3].str())};

            //  TODO open the file, compute per-band energy
            const auto energy;

            results.emplace_back(hrtf::entry<bands>{az, el, energy});
        }
    }

    return EXIT_SUCCESS;
}
