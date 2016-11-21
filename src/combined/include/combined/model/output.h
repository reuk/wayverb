#pragma once

#include "combined/model/member.h"

#include "audio_file/audio_file.h"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class persistent;
class source;
class receiver;
class capsule;

class output final : public basic_member<output> {
public:
    output& operator=(output other);

    enum class sample_rate {
        sr44_1KHz = 1,
        sr48KHz,
        sr88_2KHz,
        sr96KHz,
        sr192KHz
    };

    void set_bit_depth(audio_file::bit_depth bit_depth);
    audio_file::bit_depth get_bit_depth() const;

    void set_format(audio_file::format format);
    audio_file::format get_format() const;

    void set_sample_rate(sample_rate sample_rate);
    sample_rate get_sample_rate() const;

    void set_output_directory(std::string path);
    std::string get_output_directory() const;

    void set_unique_id(std::string unique);
    std::string get_unique_id() const;

private:
    void swap(output& other) noexcept;

    audio_file::bit_depth bit_depth_ = audio_file::bit_depth::pcm16;
    audio_file::format format_ = audio_file::format::aiff;
    sample_rate sample_rate_ = sample_rate::sr44_1KHz;

    std::string output_directory_ = "";
    std::string unique_id_ = "";
};

constexpr auto get_sample_rate(output::sample_rate sr) {
    switch (sr) {
        case output::sample_rate::sr44_1KHz: return 44100.0;
        case output::sample_rate::sr48KHz: return 48000.0;
        case output::sample_rate::sr88_2KHz: return 88200.0;
        case output::sample_rate::sr96KHz: return 96000.0;
        case output::sample_rate::sr192KHz: return 192000.0;
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string compute_output_path(const source& source,
                                const receiver& receiver,
                                const capsule& capsule,
                                const output& output);

std::vector<std::string> compute_all_file_names(const persistent& persistent,
                                                const output& output);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
