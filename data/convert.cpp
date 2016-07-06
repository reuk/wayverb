#include "sndfile.hh"

#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const std::vector<float>& data) {
    auto count = 0u;
    for (const auto& i : data) {
        os << std::setprecision(std::numeric_limits<float>::max_digits10) << i << ", ";
        if (count && !(count % 8)) {
            os << "\n";
        }
        count += 1;
    }
    return os;
}

int main(int argc, const char** argv) {
    if (argc != 2 && argc != 3) {
        throw std::runtime_error("incorrect number of args");
    }

    SndfileHandle sndfile(argv[1]);

    if (sndfile.channels() != 1) {
        throw std::runtime_error("just one channel please!");
    }

    std::vector<float> data(sndfile.frames());
    sndfile.read(data.data(), data.size());
    if (argc == 2) {
        std::cout << data;
    } else {
        std::ofstream f(argv[2]);
        f << data;
    }
}
