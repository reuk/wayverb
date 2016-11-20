#include "master.h"

#include "../../generic_slider_property.h"

#include "Application.h"

namespace left_bar {
namespace waveguide {
namespace {

class single_properties final : public PropertyPanel {
public:
    using model_t = wayverb::combined::model::single_band_waveguide;

    single_properties(model_t& model) {
        addProperties(
                {static_cast<PropertyComponent*>(new cutoff_property{model}),
                 static_cast<PropertyComponent*>(
                         new usable_portion_property{model})});
    }

private:
    class cutoff_property final : public generic_slider_property<model_t> {
    public:
        cutoff_property(model_t& model)
                : generic_slider_property{model, "cutoff / Hz", 20, 20000, 1} {
        update_from_model();
                }

    private:
        void set_model(model_t& model, const double& value) override {
            model.set_cutoff(value);
        }

        double get_model(const model_t& model) const override {
            return model.get().cutoff;
        }
    };

    class usable_portion_property final
            : public generic_slider_property<model_t> {
    public:
        usable_portion_property(model_t& model)
                : generic_slider_property{model, "usable portion", 0.1, 0.6} {
        update_from_model();
                }

    private:
        void set_model(model_t& model, const double& value) override {
            model.set_usable_portion(value);
        }

        double get_model(const model_t& model) const override {
            return model.get().usable_portion;
        }
    };
};

class multiple_properties final : public PropertyPanel {
public:
    using model_t = wayverb::combined::model::multiple_band_waveguide;

    multiple_properties(model_t& model) {
        addProperties(
                {static_cast<PropertyComponent*>(new bands_property{model}),
                 static_cast<PropertyComponent*>(new cutoff_property{model}),
                 static_cast<PropertyComponent*>(
                         new usable_portion_property{model})});
    }

private:
    class bands_property final : public generic_slider_property<model_t> {
    public:
        bands_property(model_t& model)
                : generic_slider_property{model, "bands", 1, 8, 1} {
        update_from_model();
    }

    private:
        void set_model(model_t& model, const double& value) override {
            model.set_bands(value);
        }

        double get_model(const model_t& model) const override {
            return model.get().bands;
        }
    };

    class cutoff_property final : public generic_slider_property<model_t> {
    public:
        cutoff_property(model_t& model)
                : generic_slider_property{model, "cutoff / Hz", 20, 20000, 1} {
        update_from_model();
                }

    private:
        void set_model(model_t& model, const double& value) override {
            model.set_cutoff(value);
        }

        double get_model(const model_t& model) const override {
            return model.get().cutoff;
        }
    };

    class usable_portion_property final
            : public generic_slider_property<model_t> {
    public:
        usable_portion_property(model_t& model)
                : generic_slider_property{model, "usable portion", 0.1, 0.6} {
        update_from_model();
                }

    private:
        void set_model(model_t& model, const double& value) override {
            model.set_usable_portion(value);
        }

        double get_model(const model_t& model) const override {
            return model.get().usable_portion;
        }
    };
};

}  // namespace

master::master(model_t& model)
        : TabbedComponent{TabbedButtonBar::Orientation::TabsAtTop}
        , model_{model} {
    setLookAndFeel(&wayverb_application::get_look_and_feel());

    auto single = std::make_unique<single_properties>(*model_.single_band());
    auto multi = std::make_unique<multiple_properties>(*model_.multiple_band());

    setSize(300, std::max(single->getTotalContentHeight(), multi->getTotalContentHeight()) + getTabBarDepth() + 2);
        
    addTab("single", Colours::darkgrey, single.release(), true);
    addTab("multi", Colours::darkgrey, multi.release(), true);

    const auto update_from_waveguide = [this](auto& m) {
        switch (m.get_mode()) {
            case model_t::mode::single:
                setCurrentTabIndex(0, dontSendNotification);
                break;

            case model_t::mode::multiple:
                setCurrentTabIndex(1, dontSendNotification);
                break;
        }
    };

    update_from_waveguide(model_);

    connection_ = wayverb::combined::model::waveguide::scoped_connection{
            model_.connect(update_from_waveguide)};

    initializing_ = false;
}

void master::currentTabChanged(int new_index, const String&) {
    if (!initializing_) {
        switch (new_index) {
            case 0: model_.set_mode(model_t::mode::single); break;

            case 1: model_.set_mode(model_t::mode::multiple); break;
        }
    }
}

}  // namespace waveguide
}  // namespace left_bar
