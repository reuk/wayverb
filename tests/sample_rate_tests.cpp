#include "rtaudiocommon/sample_rate_conversion.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;

TEST(sample_rate, sample_rate) {
    vector<float> input(44100);
    auto phase = 0.0f;
    for (auto & i : input) {
        i = sin(phase);
        phase += 0.1 * 2 * M_PI;
    }

    cout << "populating input" << endl;

    auto converted = convert_sample_rate(input, 500, 44100);

    cout << "completed conversion" << endl;
}
