#include "combined/model/output.h"
#include "combined/model/persistent.h"

#include "utilities/string_builder.h"

namespace wayverb {
namespace combined {
namespace model {

void output::swap(output& other) noexcept {
    using std::swap;
    swap(bit_depth_, other.bit_depth_);
    swap(format_, other.format_);
    swap(sample_rate_, other.sample_rate_);
    swap(output_directory_, other.output_directory_);
    swap(unique_id_, other.unique_id_);
}

output& output::operator=(output other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

void output::set_sample_rate(sample_rate sample_rate) {
    sample_rate_ = sample_rate;
    notify();
}

output::sample_rate output::get_sample_rate() const { return sample_rate_; }

void output::set_bit_depth(audio_file::bit_depth bit_depth) {
    bit_depth_ = bit_depth;
    notify();
}

audio_file::bit_depth output::get_bit_depth() const { return bit_depth_; }

void output::set_format(audio_file::format format) {
    format_ = format;
    notify();
}

audio_file::format output::get_format() const { return format_; }

void output::set_output_directory(std::string path) {
    output_directory_ = std::move(path);
    notify();
}

std::string output::get_output_directory() const { return output_directory_; }

void output::set_unique_id(std::string unique) {
    unique_id_ = std::move(unique);
    notify();
}

std::string output::get_unique_id() const { return unique_id_; }

////////////////////////////////////////////////////////////////////////////////

std::string compute_output_path(const source& source,
                                const receiver& receiver,
                                const capsule& capsule,
                                const output& output) {
    //  TODO platform-dependent, Windows path behaviour is different.
    return util::build_string(
            output.get_output_directory(),
            '/',
            (output.get_unique_id().empty() ? ""
                                            : (output.get_unique_id() + '.')),
            "s_",
            source.get_name().c_str(),
            ".r_",
            receiver.get_name().c_str(),
            ".c_",
            capsule.get_name().c_str(),
            '.',
            audio_file::get_extension(output.get_format()));
}

std::vector<std::string> compute_all_file_names(const persistent& persistent,
                                                const output& output) {
    std::vector<std::string> ret;
    for (const auto& source : persistent.sources()->item) {
        for (const auto& receiver : persistent.receivers()->item) {
            for (const auto& capsule : receiver->item.capsules()->item) {
                ret.emplace_back(compute_output_path(
                        source->item, receiver->item, capsule->item, output));
            }
        }
    }
    return ret;
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
