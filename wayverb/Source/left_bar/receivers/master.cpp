#include "master.h"

#include "capsules/master.h"

#include "../azimuth_elevation_property.h"
#include "../text_property.h"
#include "../vec3_property.h"

#include "combined/model/receiver.h"
#include "combined/model/capsule_presets.h"

#include "core/az_el.h"

namespace left_bar {
namespace receivers {

class receiver_properties final : public PropertyPanel {
public:
    receiver_properties(wayverb::combined::model::receiver& receiver)
            : receiver_{receiver} {
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>(
                "position", receiver_.position()->get_bounds());
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");

        auto update_from_receiver = [
            this,
            n = name.get(),
            p = position.get(),
            o = orientation.get()
        ](auto& receiver) {
            n->set(receiver.get_name());
            p->set(receiver.position()->get());
            o->set(wayverb::core::compute_azimuth_elevation(
                    receiver.get_orientation().get_pointing()));
        };

        update_from_receiver(receiver_);

        connection_ = wayverb::combined::model::receiver::scoped_connection{
                receiver_.connect(update_from_receiver)};

        name->connect_on_change(
                [this](auto&, auto name) { receiver_.set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { receiver_.position()->set(pos); });

        orientation->connect_on_change([this](auto&, auto az_el) {
            receiver_.set_orientation(wayverb::core::orientation{
                    compute_pointing(az_el)});
        });

        addProperties({name.release()});
        addProperties({position.release()});
        addProperties({orientation.release()});
    }

private:
    wayverb::combined::model::receiver& receiver_;
    wayverb::combined::model::receiver::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

class receiver_editor final : public Component {
public:
    receiver_editor(wayverb::combined::model::receiver& receiver)
            : properties_{receiver}
            , capsules_{*receiver.capsules()} {
        addAndMakeVisible(properties_);
        addAndMakeVisible(capsules_);
        addAndMakeVisible(capsule_presets_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        capsules_.setBounds(bounds.removeFromRight(150));
        capsule_presets_.setBounds(bounds.removeFromBottom(25));
        properties_.setBounds(bounds);
    }

private:
    receiver_properties properties_;
    capsules::master capsules_;
    ComboBox capsule_presets_;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> receiver_config_item::get_callout_component(
        wayverb::combined::model::receiver& model) {
    auto ret = std::make_unique<receiver_editor>(model);
    ret->setSize(500, 200);
    return std::move(ret);
}

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::combined::model::receivers& model)
        : list_box_{model, [](auto& model) {model.insert(model.end());}, [](auto& model, auto to_erase){model.erase(model.begin() + to_erase);}} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace receivers
}  // namespace left_bar
