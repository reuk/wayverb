#include "master.h"
#include "AngularLookAndFeel.h"

#include "materials/master.h"
#include "raytracer/master.h"
#include "receivers/master.h"
#include "sources/master.h"

namespace {

//  Allows a generic component to be placed in a property panel.
//  Doesn't draw a label.
template <typename T>
class wrapped_property_component final : public PropertyComponent {
public:
    template <typename... Ts>
    wrapped_property_component(int height, Ts&&... ts)
            : PropertyComponent{"empty", height}
            , content{std::forward<Ts>(ts)...} {
        setLookAndFeel(&look_and_feel_);
        addAndMakeVisible(content);
    }

    void refresh() override {}

    T content;

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
};

Array<PropertyComponent*> make_material_options(
        wayverb::combined::model::scene& scene,
        const wayverb::combined::model::app::material_presets_t& presets,
        const wayverb::combined::model::
                vector<wayverb::combined::model::material, 1>& model) {
    Array<PropertyComponent*> ret;
    size_t count = 0;
    for (const auto& i : model) {
        ret.add(new wrapped_property_component<
                left_bar::materials::config_item>(
                30, scene, presets, i.get_shared_ptr(), count++));
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
                  [this](auto state, auto progress) {
                      bottom_.set_bar_text(wayverb::combined::to_string(state));
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

    const auto item_height = 100;

    //  Populate the property panel
    property_panel_.addSection(
            "sources",
            {new wrapped_property_component<sources::master>{
                    item_height, *model_.project.persistent.sources()}});
    property_panel_.addSection(
            "receivers",
            {new wrapped_property_component<receivers::master>{
                    item_height,
                    model_.capsule_presets,
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

////////////////////////////////////////////////////////////////////////////////

// class ConfigureButton : public ButtonPropertyComponent {
// public:
//    ConfigureButton(const String& name)
//            : ButtonPropertyComponent(name, true) {
//        setLookAndFeel(&pclaf);
//    }
//
//    String getButtonText() const override { return "..."; }
//
// private:
//    PropertyComponentLAF pclaf{200};
//};
//
// class MaterialConfigureButtonComponent : public Component,
//                                         public Button::Listener {
// public:
//        MaterialConfigureButtonComponent(
//            int this_surface,
//            model::ValueWrapper<int>& shown_surface,
//            model::ValueWrapper<wayverb::core::scene_data_loader::material>&
//                    value,
//            model::ValueWrapper<util::aligned::vector<
//                    wayverb::core::scene_data_loader::material>>&
//                    preset_model)
//            : this_surface(this_surface)
//            , shown_surface(shown_surface)
//            , value(value)
//            , preset_model(preset_model) {
//        show_button.setClickingTogglesState(false);
//
//        addAndMakeVisible(show_button);
//        addAndMakeVisible(more_button);
//    }
//
//    void resized() override {
//        auto bounds = getLocalBounds();
//        show_button.setBounds(bounds.removeFromLeft(50));
//        bounds.removeFromLeft(2);
//        more_button.setBounds(bounds);
//    }
//
//    void buttonClicked(Button* b) override {
//        if (b == &show_button) {
//            shown_surface.set(
//                    shown_surface.get() == this_surface ? -1 : this_surface);
//        } else if (b == &more_button) {
//            CallOutBox::launchAsynchronously(new SurfaceComponentWithTitle(),
//                                             more_button.getScreenBounds(),
//                                             nullptr);
//        }
//    }
//
///*
//    void receive_broadcast(model::Broadcaster* b) override {
//        if (b == &shown_surface) {
//            show_button.setToggleState(shown_surface.get() == this_surface,
//                                       dontSendNotification);
//        }
//    }
//*/
//
// private:
//    TextButton show_button{"show"};
//    model::Connector<TextButton> show_connector{&show_button, this};
//
//    TextButton more_button{"..."};
//    model::Connector<TextButton> more_connector{&more_button, this};
//};
//
// class MaterialConfigureButton : public PropertyComponent,
//                                public SettableHelpPanelClient {
// public:
//    MaterialConfigureButton(
//            int this_surface,
//            model::ValueWrapper<int>& shown_surface,
//            model::ValueWrapper<wayverb::core::scene_data_loader::material>&
//                    value,
//            model::ValueWrapper<util::aligned::vector<
//                    wayverb::core::scene_data_loader::material>>&
//                    preset_model)
//            : PropertyComponent("TODO set name properly")
//            , contents(this_surface, shown_surface, value, preset_model) {
//        set_help("surface material",
//                 "Click 'show' to highlight everwhere this surface can be "
//                 "found in the model. Click the '...' button to configure how
//                 "
//                 "this surface should sound.");
//        setLookAndFeel(&pclaf);
//        addAndMakeVisible(contents);
//    }
//
//    void refresh() override {}
//
// private:
//    MaterialConfigureButtonComponent contents;
//    PropertyComponentLAF pclaf{200};
//};
//
// Array<PropertyComponent*> make_material_buttons(
//        model::ValueWrapper<int>& shown_surface,
//        const model::ValueWrapper<util::aligned::vector<
//                wayverb::core::scene_data_loader::material>>& model,
//        model::ValueWrapper<util::aligned::vector<
//                wayverb::core::scene_data_loader::material>>& preset) {
//    Array<PropertyComponent*> ret;
//    auto count = 0;
//    for (const auto& i : model) {
//        auto to_add =
//                new MaterialConfigureButton(count++, shown_surface, *i,
//                preset);
//        ret.add(to_add);
//    }
//    return ret;
//}
//
////----------------------------------------------------------------------------//
//
// class RayNumberPicker : public Component,
//                        public ComboBox::Listener,
//                        public model::BroadcastListener {
// public:
//    RayNumberPicker(model::ValueWrapper<size_t>& value)
//            : value(value) {
//        combo_box.addItem("few (1000)", 1000);
//        combo_box.addItem("some (10 000)", 10000);
//        combo_box.addItem("lots (100 000)", 100000);
//        combo_box.addItem("insane (1 000 000)", 1000000);
//
//        value_connector.trigger();
//        addAndMakeVisible(combo_box);
//    }
//
//    virtual ~RayNumberPicker() noexcept { PopupMenu::dismissAllActiveMenus();
//    }
//
//    void comboBoxChanged(ComboBox* cb) override {
//        if (cb == &combo_box) {
//            value.set(combo_box.getSelectedId());
//        }
//    }
//
//    void receive_broadcast(model::Broadcaster* cb) override {
//        if (cb == &value) {
//            auto id = std::pow(
//                    10,
//                    std::floor(std::log10(static_cast<float>(value.get()))));
//            combo_box.setSelectedId(id, dontSendNotification);
//        }
//    }
//
//    void resized() override { combo_box.setBounds(getLocalBounds()); }
//
// private:
//    model::ValueWrapper<size_t>& value;
//    model::BroadcastConnector value_connector{&value, this};
//
//    ComboBox combo_box;
//    model::Connector<ComboBox> combo_box_connector{&combo_box, this};
//};
//
// class RayNumberPickerProperty : public PropertyComponent,
//                                public SettableHelpPanelClient {
// public:
//    RayNumberPickerProperty(model::ValueWrapper<size_t>& value)
//            : PropertyComponent("rays")
//            , picker(value) {
//        set_help("ray number picker",
//                 "Choose the number of rays that should be used for the "
//                 "simulation. More rays will sound better but take longer to "
//                 "simulate.");
//        addAndMakeVisible(picker);
//    }
//
//    void refresh() override {}
//
// private:
//    RayNumberPicker picker;
//};
//
////----------------------------------------------------------------------------//
//
// class SourcesConfigureButton : public ConfigureButton {
// public:
//    SourcesConfigureButton()
//            : ConfigureButton("sources") {}
//
//    void buttonClicked() override {
//        auto cmp = std::make_unique<SourcesEditorPanel>();
//        cmp->setSize(500, 200);
//        CallOutBox::launchAsynchronously(
//                cmp.release(), getScreenBounds(), nullptr);
//    }
//
// private:
//};
//
// class ReceiversConfigureButton : public ConfigureButton {
// public:
//    ReceiversConfigureButton()
//            : ConfigureButton{"receivers"} {}
//
//    void buttonClicked() override {
//        auto cmp = std::make_unique<ReceiversEditorPanel>();
//        cmp->setSize(800, 400);
//        CallOutBox::launchAsynchronously(
//                cmp.release(), getScreenBounds(), nullptr);
//    }
//
// private:
//};
//
////----------------------------------------------------------------------------//
//
// class DebugButton : public ButtonPropertyComponent {
// public:
//    DebugButton(const std::string& name, const std::function<void()>& f)
//            : ButtonPropertyComponent("dbg", true)
//            , button_text(name)
//            , on_press(f) {}
//
// private:
//    void buttonClicked() override {
//        if (on_press) {
//            on_press();
//        }
//    }
//
//    String getButtonText() const override { return button_text; }
//
//    std::string button_text;
//    std::function<void()> on_press;
//};
//
//
////----------------------------------------------------------------------------//

// master::master(wayverb::combined::model::app& model)
//        : model_{model} {
//    set_help("configuration panel",
//             "Use the options in this panel to adjust the various settings of
//             "
//             "the simulation.");
//
//    //  Hook up the model callbacks.
//
//    model_.connect_begun([this] {
//        queue_.push([this] {
//            bottom_.set_state(BottomPanel::state::rendering);
//        });
//    });
//
//    model_.connect_engine_state([this](auto state, auto progress) {
//        queue_.push([this, state, progress] {
//            bottom_.set_bar_text(wayverb::combined::to_string(state));
//            bottom_.set_progress(progress);
//        });
//    });
//
//    model_.connect_finished([this] {
//        queue_.push([this] {
//            bottom_.set_state(BottomPanel::state::idle);
//        });
//    });
//
//    {
//        auto sources_property = std::make_unique<SourcesConfigureButton>();
//        auto receivers_property =
//        std::make_unique<ReceiversConfigureButton>();
//
//        property_panel.addSection(
//                "general",
//                {static_cast<PropertyComponent*>(sources_property.release()),
//                 static_cast<PropertyComponent*>(
//                         receivers_property.release())});
//    }
//
//        {
//            Array<PropertyComponent*> materials;
//            materials.addArray(make_material_buttons());
//            property_panel.addSection("materials", materials);
//        }
//
//        {
//            Array<PropertyComponent*> waveguide;
//            waveguide.addArray({new NumberProperty<float>(
//                                        "cutoff",
//                                        model.persistent.app.filter_frequency,
//                                        20,
//                                        20000),
//                                new NumberProperty<float>(
//                                        "oversample",
//                                        model.persistent.app.oversample_ratio,
//                                        1,
//                                        4)});
//            waveguide.add(new TextDisplayProperty<int>(
//                    "waveguide sr / Hz", 20,
//                    waveguide_sampling_rate_wrapper));
//            property_panel.addSection("waveguide", waveguide);
//        }
//
//        {
//            property_panel.addSection(
//                    "raytracer",
//                    {new RayNumberPickerProperty(model.persistent.app.rays)});
//        }
//
//        {
//            property_panel.addSection(
//                    "debug",
//                    {new DebugButton(
//                             "show closest surfaces",
//                             [&] {
//                                 listener_list.call(
//                                         &Listener::
//                                                 left_panel_debug_show_closest_surfaces,
//                                         this);
//                             }),
//                     new DebugButton(
//                             "show boundary types",
//                             [&] {
//                                 listener_list.call(
//                                         &Listener::
//                                                 left_panel_debug_show_boundary_types,
//                                         this);
//                             }),
//                     new DebugButton("hide", [&] {
//                         listener_list.call(
//                                 &Listener::left_panel_debug_hide_debug_mesh,
//       this);
//                     })});
//        }
//
//    property_panel_.setOpaque(false);
//
//    addAndMakeVisible(property_panel_);
//    addAndMakeVisible(bottom_);
//}

}  // namespace left_bar
