#include "master.h"
#include "AngularLookAndFeel.h"

#include "materials/master.h"
#include "raytracer/master.h"
#include "receivers/master.h"
#include "sources/master.h"
#include "waveguide/master.h"

namespace {

//  Allows a generic component to be placed in a property panel.
//  Doesn't draw a label.
template <typename T>
class wrapped_property_component final : public PropertyComponent {
public:
    template <typename... Ts>
    wrapped_property_component(Ts&&... ts)
            : PropertyComponent{"empty"}
            , content{std::forward<Ts>(ts)...} {
        setLookAndFeel(&look_and_feel_);
        addAndMakeVisible(content);

        setPreferredHeight(content.getHeight());
    }

    void refresh() override {}

private:
    class look_and_feel final : public AngularLookAndFeel {
    public:
        //  Don't bother drawing anything.
        void drawPropertyComponentBackground(Graphics&,
                                             int,
                                             int,
                                             PropertyComponent&) override {}
        void drawPropertyComponentLabel(Graphics&,
                                        int,
                                        int,
                                        PropertyComponent&) override {}

        //  Let the content take up the entire space.
        Rectangle<int> getPropertyComponentContentPosition(
                PropertyComponent& c) override {
            return c.getLocalBounds();
        }
    };

    look_and_feel look_and_feel_;

public:
    T content;
};

Array<PropertyComponent*> make_material_options(
        wayverb::combined::model::scene& scene,
        const wayverb::combined::model::app::material_presets_t& presets,
        const wayverb::combined::model::
                min_size_vector<wayverb::combined::model::material, 1>& model) {
    Array<PropertyComponent*> ret;
    size_t count = 0;
    for (const auto& i : model) {
        ret.add(new wrapped_property_component<
                left_bar::materials::config_item>(
                scene, presets, i.item(), count++));
    }
    return ret;
}

}  // namespace

namespace left_bar {

master::master(wayverb::combined::model::app& app, engine_message_queue& queue)
        : model_{app}
        , begun_connection_{queue.connect_begun([this] {
            //  Disable the top panel so that the user can't muck stuff up.
            property_panel_.setEnabled(false);
            bottom_.set_state(bottom::state::rendering);
        })}
        , engine_state_connection_{queue.connect_engine_state(
                  [this](auto run, auto runs, auto state, auto progress) {
                      bottom_.set_bar_text(util::build_string(
                              "run ",
                              run + 1,
                              " / ",
                              runs,
                              ": ",
                              wayverb::combined::to_string(state)));
                      bottom_.set_progress(progress);
                  })}
        , finished_connection_{queue.connect_finished([this] {
            //  Re-enable the top panel.
            property_panel_.setEnabled(true);
            bottom_.set_state(bottom::state::idle);
        })} {
    set_help("configuration panel",
             "Use the options in this panel to adjust the various settings of "
             "the simulation.");

    const auto aabb = wayverb::core::geo::compute_aabb(
            model_.project.get_scene_data().get_vertices());

    //  Populate the property panel
    property_panel_.addSection(
            "sources",
            {new wrapped_property_component<sources::master>{
                    aabb, *model_.project.persistent.sources()}});
    property_panel_.addSection(
            "receivers",
            {new wrapped_property_component<receivers::master>{
                    model_.capsule_presets,
                    aabb,
                    *model_.project.persistent.receivers()}});
    property_panel_.addSection(
            "materials",
            make_material_options(app.scene,
                                  app.material_presets,
                                  *app.project.persistent.materials()));
    property_panel_.addSection(
            "raytracer",
            {static_cast<PropertyComponent*>(new raytracer::ray_number_property{
                     *app.project.persistent.raytracer()}),
             static_cast<PropertyComponent*>(
                     new raytracer::img_src_order_property{
                             *app.project.persistent.raytracer()})});

    property_panel_.addSection(
            "waveguide",
            {new wrapped_property_component<waveguide::master>{
                    *app.project.persistent.waveguide()}});

    property_panel_.setOpaque(false);

    //  Make components visible
    addAndMakeVisible(property_panel_);
    addAndMakeVisible(bottom_);
}

void master::resized() {
    const auto bottom_height = 30;
    property_panel_.setBounds(
            getLocalBounds().withTrimmedBottom(bottom_height));
    bottom_.setBounds(getLocalBounds().removeFromBottom(bottom_height));
}

}  // namespace left_bar
