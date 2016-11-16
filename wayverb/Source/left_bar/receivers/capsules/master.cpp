#include "master.h"
#include "PolarPatternDisplay.h"

#include "../../azimuth_elevation_property.h"
#include "../../slider_property.h"

#include "combined/model/capsule.h"

#include "core/az_el.h"
#include "core/orientation.h"

namespace left_bar {
namespace receivers {
namespace capsules {

    /*
class hrtf_properties final : public PropertyPanel {
public:
    hrtf_properties(wayverb::combined::model::hrtf& model)
            : model_{model} {
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");
        auto channel = std::make_unique<choice_property>("channel");

        auto update_from_hrtf =
                [ this, o = orientation.get(), c = channel.get() ](auto& hrtf) {
            o->set(wayverb::core::compute_azimuth_elevation(
                    hrtf.get().orientation.get_pointing()));
            c->set(hrtf.get().get_channel());
        };

        update_from_hrtf(model_);

        orientation->connect_on_change([this](auto&, auto orientation) {
            model_.set_orientation(
                    wayverb::core::orientation{compute_pointing(orientation)});
        });

        channel->connect_on_change(
                [this](auto&, auto channel) { model_.setchannel(channel); });

        addProperties({orientation.release()});
        addProperties({channel.release()});
    }

private:
    wayverb::combined::model::hrtf& model_;
    wayverb::combined::model::hrtf::scoped_connection connection_;
};
    */

////////////////////////////////////////////////////////////////////////////////

class microphone_properties final : public PropertyPanel {
public:
    microphone_properties(wayverb::combined::model::microphone& model)
            : model_{model} {
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");
        auto shape = std::make_unique<slider_property>("shape", 0, 1);
        auto display = std::make_unique<PolarPatternProperty>("shape display", 80);

        auto update_from_microphone =
                [ this, o = orientation.get(), s = shape.get(), d = display.get() ](auto& mic) {
            o->set(wayverb::core::compute_azimuth_elevation(
                    mic.get().orientation.get_pointing()));
            s->set(mic.get().get_shape());
            d->set_shape(mic.get().get_shape());
        };

        update_from_microphone(model_);

        connection_ = wayverb::combined::model::microphone::scoped_connection{
                model_.connect(update_from_microphone)};

        orientation->connect_on_change([this](auto&, auto orientation) {
            model_.set_orientation(
                    wayverb::core::orientation{compute_pointing(orientation)});
        });

        shape->connect_on_change(
                [this](auto&, auto shape) { model_.set_shape(shape); });

        addProperties({orientation.release()});
        addProperties({shape.release()});
        addProperties({display.release()});
    }

private:
    wayverb::combined::model::microphone& model_;
    wayverb::combined::model::microphone::scoped_connection connection_;
};

class capsule_editor final : public TabbedComponent {
public:
    capsule_editor(wayverb::combined::model::capsule& model)
            : TabbedComponent{TabbedButtonBar::Orientation::TabsAtTop} {
        addTab("microphone",
               Colours::darkgrey,
               new microphone_properties{model.microphone()},
               true);
        //        addTab("hrtf", Colours::darkgrey, new hrtf_properties, true);
    }
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> capsule_config_item::get_callout_component(
        wayverb::combined::model::capsule& model) {
    auto ret = std::make_unique<capsule_editor>(model);
    ret->setSize(300, 200);
    return std::move(ret);
}

////////////////////////////////////////////////////////////////////////////////

master::master(
        wayverb::combined::model::vector<wayverb::combined::model::capsule, 1>&
                model)
        : list_box_{model} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace capsules
}  // namespace receivers
}  // namespace left_bar
