#include "master.h"

#include <iomanip>

namespace left_bar {
namespace materials {

class bands_component final : public Component {
public:
    bands_component() {
        for (auto& i : sliders_) {
            addAndMakeVisible(i);
        }
    }

    void set(const wayverb::core::bands_type& bands) {
        for (auto i = 0; i != wayverb::core::simulation_bands; ++i) {
            sliders_[i].setValue(bands.s[i], dontSendNotification);
        }
    }

    wayverb::core::bands_type get() const {
        wayverb::core::bands_type ret;
        for (auto i = 0; i != wayverb::core::simulation_bands; ++i) {
            ret.s[i] = sliders_[i].getValue();
        }
        return ret;
    }

    void resized() override {
        const auto bounds = getLocalBounds();

        auto total_width = [this](auto i) {
            return i * getWidth() / sliders_.size();
        };

        const auto width = total_width(1);

        for (auto i = 0u; i != sliders_.size(); ++i) {
            sliders_[i].setBounds(total_width(i), 0, width, getHeight());
        }
    }

    using on_change = util::event<wayverb::core::bands_type>;
    on_change::connection connect_on_change(on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

private:
    std::array<Slider, wayverb::core::simulation_bands> sliders_;

    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

class frequency_labels : public Component {
public:
    frequency_labels() {
        const auto centres = hrtf_data::hrtf_band_centres_hz();

        for (auto i = 0; i != wayverb::core::simulation_bands; ++i) {
            labels_[i].setText(
                    util::build_string(std::setprecision(2), centres[i]),
                    dontSendNotification);
        }

        for (auto& i : labels_) {
            addAndMakeVisible(i);
        }
    }

    void resized() override {
        const auto bounds = getLocalBounds();

        auto total_width = [this](auto i) {
            return i * getWidth() / labels_.size();
        };

        const auto width = total_width(1);

        for (auto i = 0u; i != labels_.size(); ++i) {
            labels_[i].setBounds(total_width(i), 0, width, getHeight());
        }
    }

private:
    std::array<Label, 8> labels_;
};

////////////////////////////////////////////////////////////////////////////////

class material_component final : public PropertyPanel {
public:
    using material_t = wayverb::combined::model::material;
    using presets_t = wayverb::combined::model::app::material_presets_t;

    material_component(const presets_t& presets, material_t& model)
            : presets_{presets}
            , model_{model} {}

private:
    const presets_t& presets_;
    material_t& model_;
};

////////////////////////////////////////////////////////////////////////////////

class material_component_with_title final : public Component {
public:
    using material_t = wayverb::combined::model::material;
    using presets_t = wayverb::combined::model::app::material_presets_t;

    material_component_with_title(const presets_t& presets, material_t& model)
            : presets_{presets}
            , model_{model}
            , label_{"", model_.get_name()}
            , material_component_{presets, model_} {
        addAndMakeVisible(label_);
        addAndMakeVisible(material_component_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label_.setBounds(bounds.removeFromTop(25));
        material_component_.setBounds(bounds);
    }

private:
    const presets_t& presets_;
    material_t& model_;

    Label label_;
    material_component material_component_;
};

////////////////////////////////////////////////////////////////////////////////

config_item::config_item(wayverb::combined::model::scene& scene,
                         const presets_t& presets,
                         std::shared_ptr<material_t> model,
                         size_t index)
        : scene_{scene}
        , scene_connection_{scene_.connect_visible_surface_changed(
                  [this](auto surf) {
                      show_button_.setToggleState(surf && *surf == index_,
                                                  dontSendNotification);
                  })}
        , presets_{presets}
        , model_{std::move(model)}
        , index_{index}
        , label_{"", model_->get_name()} {
    addAndMakeVisible(label_);
    addAndMakeVisible(show_button_);
    addAndMakeVisible(config_button_);
}

void config_item::resized() {
    const auto button_width = this->getHeight();
    auto bounds = this->getLocalBounds();
    config_button_.setBounds(
            bounds.removeFromRight(button_width).reduced(2, 2));
    show_button_.setBounds(
            bounds.removeFromRight(button_width * 2).reduced(2, 2));
    label_.setBounds(bounds.reduced(2, 2));
}

void config_item::buttonClicked(Button* b) {
    if (b == &show_button_) {
        scene_.set_visible_surface(
                b->getToggleState() ? std::experimental::nullopt
                                    : std::experimental::make_optional(index_));
    } else if (b == &config_button_) {
        CallOutBox::launchAsynchronously(
                new material_component_with_title{presets_, *model_},
                config_button_.getScreenBounds(),
                nullptr);
    }
}

}  // namespace materials
}  // namespace left_bar
