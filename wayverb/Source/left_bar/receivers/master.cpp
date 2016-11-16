#include "master.h"

#include "../azimuth_elevation_property.h"
#include "../text_property.h"
#include "../vec3_property.h"

#include "combined/model/app.h"

namespace left_bar {
namespace receivers {

class receiver_properties final : public PropertyPanel {
public:
    receiver_properties(wayverb::combined::model::receiver& receiver)
            : receiver_{receiver} {
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>(
                "position", receiver_.position().get_bounds());
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");

        auto update_from_receiver = [
            this,
            n = name.get(),
            p = position.get(),
            o = orientation.get()
        ](auto& receiver) {
            n->set(receiver.get_name());
            p->set(receiver.position().get());
            o->set(wayverb::core::compute_azimuth_elevation(
                    receiver.get_orientation().get_pointing()));
        };

        update_from_receiver(receiver_);

        connection_ = wayverb::combined::model::receiver::scoped_connection{
                receiver_.connect(update_from_receiver)};

        name->connect_on_change(
                [this](auto&, auto name) { receiver_.set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { receiver_.position().set(pos); });

        orientation->connect_on_change([this](auto&, auto az_el) {
            receiver_.set_orientation(wayverb::core::orientation{
                    wayverb::core::compute_pointing(az_el)});
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
            : properties_{receiver} {
        addAndMakeVisible(properties_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromRight(150);
        properties_.setBounds(bounds);
    }

private:
    receiver_properties properties_;
    //  TODO capsules::master capsules_;
};

////////////////////////////////////////////////////////////////////////////////

Component* receiver_config_item::get_callout_component(
        wayverb::combined::model::receiver& model) {
    auto ret = std::make_unique<receiver_editor>(model);
    ret->setSize(500, 200);
    return ret.release();
}

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> make_receiver_editor(
wayverb::combined::model::receiver& receiver) {
    return std::make_unique<receiver_editor>(receiver);
}

master::master(wayverb::combined::model::app& app)
        : model_{app}
        , list_box_{model_.project.persistent.receivers()} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace receivers
}  // namespace left_bar
