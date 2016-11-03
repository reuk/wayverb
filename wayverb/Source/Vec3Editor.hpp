#pragma once

#include "HelpWindow.hpp"
#include "ValueWrapperSlider.hpp"

#include "UtilityComponents/NumberEditor.hpp"

#include "glm/glm.hpp"

#include <array>
#include <iomanip>
#include <sstream>

/*
template <typename T>
class NumberProperty : public PropertyComponent,
                       public NumberEditor<T>::Listener,
                       public model::BroadcastListener {
public:
    NumberProperty(const String& name,
                   model::ValueWrapper<T>& value,
                   T min,
                   T max)
            : PropertyComponent(name)
            , value(value) {
        addAndMakeVisible(editor);
        editor.set_clipping(Range<T>(min, max));
        value_connector.trigger();
    }

    void refresh() override {
    }

    void number_editor_value_changed(NumberEditor<T>* t) override {
        if (t == &editor) {
            value.set(editor.get_value());
        }
    }

    void receive_broadcast(model::Broadcaster* cb) override {
        if (cb == &value) {
            editor.set_value(value.get(), false);
        }
    }

    void set_minimum(T u) {
        editor.set_minimum(u);
    }

    void set_maximum(T u) {
        editor.set_maximum(u);
    }

private:
    model::ValueWrapper<T>& value;
    model::BroadcastConnector value_connector{&value, this};

    NumberEditor<T> editor;
    model::Connector<NumberEditor<T>> editor_connector{&editor, this};
};

class Vec3Editor : public Component {
public:
    Vec3Editor(model::ValueWrapper<glm::vec3>& value,
               const glm::vec3& min,
               const glm::vec3& max);
    void resized() override;

private:
    PropertyPanel property_panel;
};

class Vec3Property : public PropertyComponent, public SettableHelpPanelClient {
public:
    Vec3Property(const String& name,
                 model::ValueWrapper<glm::vec3>& value,
                 const glm::vec3& min,
                 const glm::vec3& max);
    void refresh() override;

private:
    Vec3Editor editor;
};
*/
