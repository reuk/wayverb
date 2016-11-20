#include "master.h"

#include "utilities/string_builder.h"

#include "../AngularLookAndFeel.h"
#include "../UtilityComponents/connector.h"
#include "../generic_combo_box_property.h"
#include "../text_property.h"

namespace output {

namespace {

class bit_depth_property final
        : public generic_combo_box_property<wayverb::combined::model::output,
                                            audio_file::bit_depth> {
public:
    bit_depth_property(model_t& model)
            : generic_combo_box_property{
                      model,
                      "bit depth",
                      {audio_file::bit_depth::pcm16,
                       audio_file::bit_depth::pcm24,
                       audio_file::bit_depth::pcm32,
                       audio_file::bit_depth::float32},
                      [](auto e) { return audio_file::get_description(e); }} {}

private:
    void set_model(model_t& model, const value_t& e) override {
        model.set_bit_depth(e);
    }

    value_t get_model(const model_t& model) const override {
        return model.get_bit_depth();
    }
};

class sample_rate_property final
        : public generic_combo_box_property<
                  wayverb::combined::model::output,
                  wayverb::combined::model::output::sample_rate> {
public:
    sample_rate_property(model_t& model)
            : generic_combo_box_property{
                      model,
                      "sample rate",
                      {model_t::sample_rate::sr44_1KHz,
                       model_t::sample_rate::sr48KHz,
                       model_t::sample_rate::sr88_2KHz,
                       model_t::sample_rate::sr96KHz,
                       model_t::sample_rate::sr192KHz},
                      [](auto i) {
                          return util::build_string(
                                  wayverb::combined::model::get_sample_rate(i) /
                                          1000.0,
                                  "KHz");
                      }} {}

private:
    void set_model(model_t& model, const value_t& e) override {
        model.set_sample_rate(e);
    }

    value_t get_model(const model_t& model) const override {
        return model.get_sample_rate();
    }
};

class format_property final
        : public generic_combo_box_property<wayverb::combined::model::output,
                                            audio_file::format> {
public:
    format_property(model_t& model)
            : generic_combo_box_property{
                      model,
                      "format",
                      {audio_file::format::aiff, audio_file::format::wav},
                      [](auto e) { return audio_file::get_extension(e); }} {}

private:
    void set_model(model_t& model, const value_t& e) override {
        model.set_format(e);
    }

    value_t get_model(const model_t& model) const override {
        return model.get_format();
    }
};

class directory_component final : public Component, public ButtonListener {
public:
    using model_t = wayverb::combined::model::output;

    directory_component(model_t& model)
            : model_{model}
            , connection_{model_.connect([this](auto&) {
                label_.setText(model_.get_output_directory(),
                               dontSendNotification);
            })} {
        model_.set_output_directory(
                File::getSpecialLocation(
                        File::SpecialLocationType::userHomeDirectory)
                        .getFullPathName()
                        .toStdString());

        addAndMakeVisible(label_);
        addAndMakeVisible(text_button_);
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(2, 2);
        const auto button_height = bounds.getHeight();
        text_button_.setBounds(bounds.removeFromRight(
                text_button_.getBestWidthForHeight(button_height)));
        label_.setBounds(bounds);
    }

    void buttonClicked(Button* b) override {
        FileChooser fc{"select output directory...",
                       File{model_.get_output_directory()}};
        if (fc.browseForDirectory()) {
            model_.set_output_directory(
                    fc.getResult().getFullPathName().toStdString());
        }
    }

private:
    model_t& model_;
    Label label_;
    TextButton text_button_{"browse..."};
    model::Connector<TextButton> text_button_connector_{&text_button_, this};

    model_t::scoped_connection connection_;
};

class directory_property final : public PropertyComponent {
public:
    using model_t = wayverb::combined::model::output;
    directory_property(model_t& model)
            : PropertyComponent{"output directory"}
            , directory_component_{model} {
        addAndMakeVisible(directory_component_);
    }

    void refresh() override {}

private:
    directory_component directory_component_;
};

}  // namespace

class config final : public PropertyPanel {
public:
    using model_t = wayverb::combined::model::output;

    config(model_t& model)
            : model_{model} {
        addProperties({new directory_property{model_}});

        auto name_property = new text_property{"name"};
        name_property->connect_on_change(
                [this](auto&, auto str) { model_.set_unique_id(str); });

        name_property->editor.setTextToShowWhenEmpty(
                "this will be used as a prefix for output files",
                Colours::grey);

        connection_ = model_t::scoped_connection{
                model_.connect([this, name_property](auto&) {
                    name_property->set(model_.get_unique_id());
                })};

        addProperties({name_property});

        addProperties({new bit_depth_property{model_}});
        addProperties({new sample_rate_property{model_}});
        addProperties({new format_property{model_}});

        model_.notify();

        setSize(600, getTotalContentHeight());
    }

private:
    model_t& model_;
    model_t::scoped_connection connection_;
};

void get_output_options(wayverb::combined::model::output& model,
                        modal_callback callback) {
    begin_modal_dialog(
            "configure output",
            make_ok_cancel_window_ptr(std::make_unique<config>(model)),
            std::move(callback));
}

}  // namespace output
