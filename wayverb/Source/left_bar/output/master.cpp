#include "master.h"

#include "utilities/string_builder.h"

namespace left_bar {
namespace output {

master::master(model_t& model)
        : model_{model}
        , bit_depth_{new generic_property_component<ComboBox>{"bit depth", 25}}
        , sample_rate_{new generic_property_component<ComboBox>{"sample rate",
                                                                25}}
        , format_{new generic_property_component<ComboBox>{"format", 25}} {
    for (auto i : {audio_file::bit_depth::pcm16,
                   audio_file::bit_depth::pcm24,
                   audio_file::bit_depth::pcm32,
                   audio_file::bit_depth::float32}) {
        bit_depth_->content.addItem(audio_file::get_description(i),
                                    static_cast<int>(i));
    }
    bit_depth_->content.addListener(this);

    for (auto i : {model_t::sample_rate::sr44_1KHz,
                   model_t::sample_rate::sr48KHz,
                   model_t::sample_rate::sr88_2KHz,
                   model_t::sample_rate::sr96KHz,
                   model_t::sample_rate::sr192KHz}) {
        sample_rate_->content.addItem(
                util::build_string(
                        wayverb::combined::model::get_sample_rate(i) / 1000.0,
                        "KHz"),
                static_cast<int>(i));
    }
    sample_rate_->content.addListener(this);

    for (auto i : {audio_file::format::aiff, audio_file::format::wav}) {
        format_->content.addItem(audio_file::get_extension(i),
                                 static_cast<int>(i));
    }
    format_->content.addListener(this);

    const auto update_from_model = [this](auto& model) {
        bit_depth_->content.setSelectedId(
                static_cast<int>(model.get_bit_depth()));
        sample_rate_->content.setSelectedId(
                static_cast<int>(model.get_sample_rate()));
        format_->content.setSelectedId(static_cast<int>(model.get_format()));
    };

    update_from_model(model_);

    connection_ = model_t::scoped_connection{model_.connect(update_from_model)};

    addProperties({bit_depth_, sample_rate_, format_});
}

void master::comboBoxChanged(ComboBox* cb) {
    if (cb == &bit_depth_->content) {
        model_.set_bit_depth(static_cast<audio_file::bit_depth>(
                bit_depth_->content.getSelectedId()));
    } else if (cb == &sample_rate_->content) {
        model_.set_sample_rate(static_cast<model_t::sample_rate>(
                sample_rate_->content.getSelectedId()));
    } else if (cb == &format_->content) {
        // model.set_format(format_->getSelectedId());
    }
}

}  // namespace output
}  // left_bar
