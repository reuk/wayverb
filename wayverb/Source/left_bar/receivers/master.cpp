#include "master.h"

#include "capsules/master.h"

#include "../azimuth_elevation_property.h"
#include "../vec3_property.h"
#include "text_property.h"

#include "combined/model/receiver.h"

#include "core/az_el.h"

namespace left_bar {
namespace receivers {

class receiver_properties final : public PropertyPanel {
public:
    receiver_properties(wayverb::core::geo::box aabb,
                        wayverb::combined::model::receiver& receiver)
            : aabb_{std::move(aabb)}
            , receiver_{receiver} {
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>("position", aabb_);
        auto orientation =
                std::make_unique<azimuth_elevation_property>("orientation");

        const auto update_from_receiver = [
            this,
            n = name.get(),
            p = position.get(),
            o = orientation.get()
        ](auto& receiver) {
            n->set(receiver.get_name());
            p->set(receiver.get_position());
            o->set(wayverb::core::compute_azimuth_elevation(
                    receiver.get_orientation().get_pointing()));
        };

        update_from_receiver(receiver_);

        connection_ = wayverb::combined::model::receiver::scoped_connection{
                receiver_.connect(update_from_receiver)};

        name->connect_on_change(
                [this](auto&, auto name) { receiver_.set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { receiver_.set_position(pos); });

        orientation->connect_on_change([this](auto&, auto az_el) {
            receiver_.set_orientation(
                    wayverb::core::orientation{compute_pointing(az_el)});
        });

        addProperties({name.release()});
        addProperties({position.release()});
        addProperties({orientation.release()});
    }

private:
    wayverb::core::geo::box aabb_;
    wayverb::combined::model::receiver& receiver_;
    wayverb::combined::model::receiver::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

class receiver_editor final : public Component, public ComboBox::Listener {
public:
    receiver_editor(
            const wayverb::combined::model::app::capsule_presets_t& presets,
            wayverb::core::geo::box aabb,
            std::shared_ptr<wayverb::combined::model::receiver> receiver)
            : receiver_{std::move(receiver)}
            , properties_{std::move(aabb), *receiver_}
            , capsules_{*receiver_->capsules()}
            , presets_{presets} {
        combo_box_.setTextWhenNothingSelected("capsule group presets...");

        {
            auto count = 1;
            for (const auto& i : presets_) {
                combo_box_.addItem(i.name, count++);
            }
        }

        addAndMakeVisible(properties_);
        addAndMakeVisible(capsules_);
        addAndMakeVisible(combo_box_);

        setSize(500, properties_.getTotalContentHeight());
    }

    void resized() override {
        auto bounds = getLocalBounds();
        auto capsule_bounds = bounds.removeFromRight(150);
        properties_.setBounds(bounds);

        combo_box_.setBounds(capsule_bounds.removeFromBottom(25));
        capsules_.setBounds(capsule_bounds);
    }

    void comboBoxChanged(ComboBox* cb) override {
        const auto selected = cb->getSelectedItemIndex();
        if (selected != -1) {
            const auto& capsules = presets_[selected].capsules;
            receiver_->capsules()->clear();
            for (const auto& capsule : capsules) {
                receiver_->capsules()->insert(receiver_->capsules()->end(),
                                              capsule);
            }
        }
        cb->setSelectedItemIndex(-1, dontSendNotification);
    }

private:
    std::shared_ptr<wayverb::combined::model::receiver> receiver_;

    receiver_properties properties_;
    capsules::master capsules_;
    ComboBox combo_box_;
    model::Connector<ComboBox> capsule_presets_connector_{&combo_box_, this};

    wayverb::combined::model::app::capsule_presets_t presets_;
};

////////////////////////////////////////////////////////////////////////////////

master::master(const wayverb::combined::model::app::capsule_presets_t& presets,
               wayverb::core::geo::box aabb,
               wayverb::combined::model::receivers& receivers)
        : list_box_{receivers,
                    [&presets, aabb](auto shared) {
                        return make_list_config_item_ptr(
                                shared, [&presets, aabb](auto shared) {
                                    return std::make_unique<receiver_editor>(
                                            presets, aabb, shared);
                                });
                    },
                    [](auto& model) {
                        model.insert(model.end(),
                                     wayverb::combined::model::receiver{});
                    }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);

    setSize(300, 100);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace receivers
}  // namespace left_bar
