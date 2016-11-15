#include "master.h"
#include "../list_config_item.h"

#include "combined/model/app.h"

namespace left_bar {
namespace sources {

class slider_property final : public PropertyComponent , public Slider::Listener {
public:
    slider_property(const String& name, double min, double max)
            : PropertyComponent{name, 25}
            , slider_{Slider::SliderStyle::IncDecButtons, Slider::TextEntryBoxPosition::TextBoxLeft} {
        slider_.setIncDecButtonsMode(Slider::IncDecButtonMode::incDecButtonsDraggable_AutoDirection);
        slider_.setRange(min, max);
        addAndMakeVisible(slider_);
    }

    double get() const { return slider_.getValue(); }
    void set(double x) { slider_.setValue(x, dontSendNotification); }

    void refresh() override {}

    using on_change = util::event<slider_property&, double>;
    on_change::connection connect_on_change(on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

    void sliderValueChanged(Slider* s) override {
        on_change_(*this, get());
    }

private:
    Slider slider_;
    model::Connector<Slider> slider_connector_{&slider_, this};
    on_change on_change_;
};

class vec3_editor final : public PropertyPanel {
public:
    vec3_editor(const util::range<glm::vec3>& range) {
        auto x = std::make_unique<slider_property>("x", range.get_min().x, range.get_max().x);
        auto y = std::make_unique<slider_property>("y", range.get_min().y, range.get_max().y);
        auto z = std::make_unique<slider_property>("z", range.get_min().z, range.get_max().z);

        const auto callback = [this](auto&, auto val) {
            on_change_(*this, get());
        };

        x->connect_on_change(callback);
        y->connect_on_change(callback);
        z->connect_on_change(callback);

        x_ = x.release();
        y_ = y.release();
        z_ = z.release();
        addProperties({x_, y_, z_});
    }

    using on_change = util::event<vec3_editor&, glm::vec3>;
    on_change::connection connect_on_change(
            on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

    glm::vec3 get() const {
        return glm::vec3(x_->get(), y_->get(), z_->get());
    }

    void set(const glm::vec3& v) {
        x_->set(v.x);
        y_->set(v.y);
        z_->set(v.z);
    }

private:
    slider_property * x_;
    slider_property * y_;
    slider_property * z_;
    on_change on_change_;
};

class vec3_property final : public PropertyComponent {
public:
    vec3_property(const String& name, const util::range<glm::vec3>& range)
            : PropertyComponent{name, 79}
            , editor_{range} {

        editor_.connect_on_change([this](auto&, auto vec) {
            on_change_(*this, vec);
        });
            
        addAndMakeVisible(editor_);
    }

    using on_change = util::event<vec3_property&, glm::vec3>;
    on_change::connection connect_on_change(on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

    void refresh() override {}

    glm::vec3 get() const { return editor_.get(); }
    void set(const glm::vec3& v) { editor_.set(v); }

private:
    vec3_editor editor_;
    on_change on_change_;
};

class name_property final : public PropertyComponent , public TextEditor::Listener {
public:
    name_property(const String& name)
            : PropertyComponent{name, 25} {
        addAndMakeVisible(editor_);
    }

    std::string get() const { return editor_.getText().toStdString(); }
    void set(const std::string& s) { editor_.setText(s, false); }

    void refresh() override {}

    using on_change = util::event<name_property&, std::string>;
    on_change::connection connect_on_change(on_change::callback_type callback) {
        return on_change_.connect(std::move(callback));
    }

    void textEditorReturnKeyPressed(TextEditor&) override {
        on_change_(*this, get());
    }

private:
    TextEditor editor_;
    model::Connector<TextEditor> editor_connector_{&editor_, this};
    on_change on_change_;
};

class source_editor final : public PropertyPanel {
public:
    source_editor(wayverb::combined::model::source& source)
            : source_{source} {
        //  Make properties.
        auto name = std::make_unique<name_property>("name");
        auto position = std::make_unique<vec3_property>(
                        "position",
                        source.position().get_bounds());

        auto update_from_source = [this, n = name.get(), p = position.get()] (auto& source){
            n->set(source.get_name());
            p->set(source.position().get());
        };

        update_from_source(source_);

        //  Tell UI objects what to do when the data source changes.
        connection_ = wayverb::combined::model::source::scoped_connection{
                source_.connect(update_from_source)};

        //  Tell model what to do when the ui is updated by the user.
        name->connect_on_change([this] (auto&, auto name) {
            source_.set_name(name);
        });

        position->connect_on_change([this] (auto&, auto pos) {
            source_.position().set(pos);
        });

        addProperties({name.release()});
        addProperties({position.release()});
        refreshAll();
    }

private:
    wayverb::combined::model::source& source_;
    wayverb::combined::model::source::scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

Component* source_config_item::get_callout_component(wayverb::combined::model::source& model) {
    auto ret = std::make_unique<source_editor>(model);
    ret->setSize(250, ret->getTotalContentHeight());
    return ret.release();
}

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Component> make_source_editor(wayverb::combined::model::source& source) {
    return std::make_unique<source_editor>(source);
}
master::master(wayverb::combined::model::app& app)
        : model_{app}
        , list_box_{model_.project.persistent.sources()} {
    list_box_.setRowHeight(30);
    addAndMakeVisible(list_box_);
}

void master::resized() { list_box_.setBounds(getLocalBounds()); }

}  // namespace sources
}  // namespace left_bar
