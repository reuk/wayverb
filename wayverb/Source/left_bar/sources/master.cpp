#include "master.h"
#include "../list_config_item.h"

#include "../vec3_property.h"
#include "text_property.h"

#include "combined/model/source.h"

namespace left_bar {
namespace sources {

class source_editor final : public PropertyPanel {
public:
    source_editor(wayverb::core::geo::box aabb,
                  std::shared_ptr<wayverb::combined::model::source> source)
            : aabb_{std::move(aabb)}
            , source_{std::move(source)} {
        //  Make properties.
        auto name = std::make_unique<text_property>("name");
        auto position = std::make_unique<vec3_property>("position", aabb_);

        auto update_from_source =
                [ this, n = name.get(), p = position.get() ](auto& source) {
            n->set(source.get_name());
            p->set(source.get_position());
        };

        update_from_source(*source_);

        //  Tell UI objects what to do when the data source changes.
        connection_ = wayverb::combined::model::source::scoped_connection{
                source_->connect(update_from_source)};

        //  Tell model what to do when the ui is updated by the user.
        name->connect_on_change(
                [this](auto&, auto name) { source_->set_name(name); });

        position->connect_on_change(
                [this](auto&, auto pos) { source_->set_position(pos); });

        addProperties({name.release()});
        addProperties({position.release()});

        setSize(300, getTotalContentHeight());
    }

private:
    wayverb::core::geo::box aabb_;
    std::shared_ptr<wayverb::combined::model::source> source_;
    wayverb::combined::model::source::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

master::master(wayverb::core::geo::box aabb,
               wayverb::combined::model::sources& model)
        : list_box_{model,
                    [aabb](auto shared) {
                        return make_list_config_item_ptr(
                                shared, [aabb](auto shared) {
                                    return std::make_unique<source_editor>(
                                            aabb, shared);
                                },
                                "source");
                    },
                    [aabb](auto& model) {
                        wayverb::combined::model::source to_insert{};
                        to_insert.set_position(centre(aabb));
                        model.insert(model.end(), to_insert);
                    }} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);

    setSize(300, 100);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace sources
}  // namespace left_bar
