#include "master.h"

namespace left_bar {
namespace output {

master::master(model_t& model)
        : model_{model}
        , bit_depth_{new generic_property_component<ComboBox>{"bit depth", 25}}
        , sample_rate_{new generic_property_component<ComboBox>{"sample rate",
                                                                25}}
        , format_{new generic_property_component<ComboBox>{"format", 25}} {
    bit_depth_->content.addItem("16", model_t::bit_depth::bd16);
    bit_depth_->content.addItem("24", model_t::bit_depth::bd24);
    bit_depth_->content.addListener(this);

    sample_rate_->content.addItem("44.1KHz", model_t::sample_rate::sr44_1KHz);
    sample_rate_->content.addItem("48.0KHz", model_t::sample_rate::sr48KHz);
    sample_rate_->content.addItem("88.2KHz", model_t::sample_rate::sr88_2KHz);
    sample_rate_->content.addItem("96.0KHz", model_t::sample_rate::sr96KHz);
    sample_rate_->content.addItem("192KHz", model_t::sample_rate::sr192KHz);
    sample_rate_->content.addListener(this);

    const auto update_from_model = [this](auto& model) {
        bit_depth_->content.setSelectedId(model.get_bit_depth());
        sample_rate_->content.setSelectedId(model.get_sample_rate());
    };

    update_from_model(model_);

    connection_ = model_t::scoped_connection{model_.connect(update_from_model)};

    addProperties({bit_depth_, sample_rate_, format_});
}

void master::comboBoxChanged(ComboBox* cb) {
    if (cb == &bit_depth_->content) {
        model_.set_bit_depth(static_cast<model_t::bit_depth>(
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
