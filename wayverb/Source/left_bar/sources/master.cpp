#include "master.h"
#include "../list_config_item.h"

#include "combined/model/app.h"

namespace left_bar {
namespace sources {

class vec3_editor final : public PropertyPanel, public Value::Listener {
public:
    vec3_editor(const util::range<glm::vec3>& range) {
        addProperties({
            new SliderPropertyComponent{x_, "x", range.get_min().x, range.get_max().x, 0.0},
            new SliderPropertyComponent{y_, "y", range.get_min().y, range.get_max().y, 0.0},
            new SliderPropertyComponent{z_, "z", range.get_min().z, range.get_max().z, 0.0},
        });
    }

    using value_changed = util::event<glm::vec3>;
    value_changed::connection connect_value_changed(value_changed::callback_type callback) {
        return value_changed_.connect(std::move(callback));
    }

    void valueChanged(Value&) override {
        value_changed_(glm::vec3(x_.getValue(), y_.getValue(), z_.getValue()));
    }

private:
    Value x_;
    Value y_;
    Value z_;
    
    util::event<glm::vec3> value_changed_;
};

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::combined::model::app& app)
        : model_{app}
        , list_box_{model_.project.persistent.sources(),
                    [this](int row, bool selected) {
                        auto ret = std::make_unique<list_config_item>(
                            [] {return std::unique_ptr<Component>();}
                        );
                        const auto &sources = model_.project.persistent.sources();
                        jassert(0 <= row && row < sources.size());
                        ret->set_label(sources[row].get_name());
                        ret->setInterceptsMouseClicks(false, true);
                        return ret;
                    }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() {
    list_box_.setBounds(getLocalBounds());
}

}  // namespace sources
}  // namespace left_bar
