#pragma once

#include "common/aligned/vector.h"
#include "common/cl/scene_structs.h"
#include "common/hrtf.h"

enum class hrtf_channel { left, right };

template <typename T>
void for_each_band(double sample_rate, T callback) {
    for (auto i = 0u; i != hrtf_data::edges.size() - 1 &&
                      hrtf_data::edges[i] < sample_rate * 0.5;
         ++i) {
        callback(i, hrtf_data::edges[i + 0], hrtf_data::edges[i + 1]);
    }
}

/// Given a vector of frequency band signals, a vector of band edges, and
/// a sampling rate, filter the bands and sum them down.

aligned::vector<float> mixdown(
        const aligned::vector<volume_type>& data);
aligned::vector<aligned::vector<float>> mixdown(
        const aligned::vector<aligned::vector<volume_type>>& data);

aligned::vector<aligned::vector<float>> multiband_filter(
        const aligned::vector<aligned::vector<float>> bands,
        const aligned::vector<double>& band_edges,
        double sample_rate);

aligned::vector<float> multiband_filter_and_mixdown(
        aligned::vector<volume_type> bands, double sample_rate);
