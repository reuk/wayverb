#pragma once

#include "common/aligned/vector.h"
#include "common/cl/scene_structs.h"
#include "common/hrtf.h"

enum class hrtf_channel { left, right };

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
