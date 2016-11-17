#include "master.h"
#include "../list_config_item.h"

#include "../text_property.h"
#include "../vec3_property.h"

#include "combined/model/source.h"

namespace left_bar {
namespace sources {

class source_editor final : public PropertyPanel {
public:
    source_editor(std::shared_ptr<wayverb::combined::model::source> source)
            : source_{std::move(source)} {
        //  Make properties.
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>(
                "position", source_->position()->get_bounds());

        auto update_from_source =
                [ this, n = name.get(), p = position.get() ](auto& source) {
            n->set(source.get_name());
            p->set(source.position()->get());
        };

        update_from_source(*source_);

        //  Tell UI objects what to do when the data source changes.
        connection_ = wayverb::combined::model::source::scoped_connection{
                source_->connect(update_from_source)};

        //  Tell model what to do when the ui is updated by the user.
        name->connect_on_change(
                [this](auto&, auto name) { source_->set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { source_->position()->set(pos); });

        addProperties({name.release()});
        addProperties({position.release()});

        setSize(300, getTotalContentHeight());
    }

private:
    std::shared_ptr<wayverb::combined::model::source> source_;
    wayverb::combined::model::source::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::combined::model::sources& model)
        : list_box_{model,
        [] (auto shared) {
            return std::make_unique<list_config_item<wayverb::combined::model::source>>(shared, [](auto shared) {
                return std::make_unique<source_editor>(shared);
            });
        },
        [] (auto& model) {
            model.insert(model.end());
        }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace sources
}  // namespace left_bar
