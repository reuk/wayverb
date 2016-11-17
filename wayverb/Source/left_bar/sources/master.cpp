#include "master.h"
#include "../list_config_item.h"

#include "../text_property.h"
#include "../vec3_property.h"

#include "combined/model/source.h"

namespace left_bar {
namespace sources {

class source_editor final : public PropertyPanel {
public:
    source_editor(wayverb::combined::model::source& source)
            : source_{source} {
        //  Make properties.
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>(
                "position", source_.position()->get_bounds());

        auto update_from_source =
                [ this, n = name.get(), p = position.get() ](auto& source) {
            n->set(source.get_name());
            p->set(source.position()->get());
        };

        update_from_source(source_);

        //  Tell UI objects what to do when the data source changes.
        connection_ = wayverb::combined::model::source::scoped_connection{
                source_.connect(update_from_source)};

        //  Tell model what to do when the ui is updated by the user.
        name->connect_on_change(
                [this](auto&, auto name) { source_.set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { source_.position()->set(pos); });

        addProperties({name.release()});
        addProperties({position.release()});
    }

private:
    wayverb::combined::model::source& source_;
    wayverb::combined::model::source::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> source_config_item::get_callout_component(
        wayverb::combined::model::source& model) {
    auto ret = std::make_unique<source_editor>(model);
    ret->setSize(300, ret->getTotalContentHeight());
    return std::move(ret);
}

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::combined::model::sources& model)
        : list_box_{model, [] (auto& model) {
            model.insert(model.end());
        }, [] (auto& model, auto to_erase) {
            model.erase(model.begin() + to_erase);
        }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace sources
}  // namespace left_bar
