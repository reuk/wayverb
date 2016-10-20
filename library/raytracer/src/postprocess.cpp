#include "raytracer/postprocess.h"

#include "common/dsp_vector_ops.h"

#include "utilities/decibels.h"
#include "utilities/map_to_vector.h"

namespace raytracer {

/// Keep tracing until power has fallen by 60 decibels.
size_t compute_optimum_reflection_number(double absorption) {
    return std::ceil(-3 / std::log10(1 - absorption));
}

/// Find the index of the last sample with an amplitude of minVol or higher,
/// then resize the vectors down to this length.
void trimTail(aligned::vector<aligned::vector<float>>& audioChannels,
              float minVol) {
    using index_type = std::common_type_t<
            std::iterator_traits<
                    aligned::vector<float>::reverse_iterator>::difference_type,
            int>;

    // Find last index of required amplitude or greater.
    const auto len = std::accumulate(
            begin(audioChannels),
            end(audioChannels),
            0,
            [minVol](auto current, const auto& i) {
                return std::max(
                        index_type{current},
                        index_type{
                                distance(i.begin(),
                                         std::find_if(i.rbegin(),
                                                      i.rend(),
                                                      [minVol](auto j) {
                                                          return std::abs(j) >=
                                                                 minVol;
                                                      })
                                                 .base()) -
                                1});
            });

    // Resize.
    for (auto&& i : audioChannels)
        i.resize(len);
}

}  // namespace raytracer
