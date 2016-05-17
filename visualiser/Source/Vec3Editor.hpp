#pragma once

#include "ModelWrapper.hpp"
#include "ValueWrapperSlider.hpp"

#include <array>
#include <iomanip>
#include <sstream>

/*
template <typename T>
class NumberEditor : public ValueWrapperSlider<T> {
public:
    NumberEditor(model::ValueWrapper<T>& value)
            : ValueWrapperSlider<T>(value) {
        this->changeListenerCallback(&value);
    }
};

template <typename T>
class NumberProperty : public PropertyComponent {
public:
    NumberProperty(const String& name, model::ValueWrapper<T>& value)
            : PropertyComponent(name)
            , editor(value) {
        addAndMakeVisible(editor);
    }
    void refresh() override {
    }

private:
    NumberEditor<T> editor;
};
*/

template <typename T>
class NumberEditor : public Component,
                     public TextEditor::Listener,
                     public Button::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener& rhs) = default;
        Listener& operator=(const Listener& rhs) = default;
        Listener(Listener&& rhs) noexcept = default;
        Listener& operator=(Listener&& rhs) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void number_editor_value_changed(NumberEditor& e) = 0;
    };

    NumberEditor()
            : text_editor()
            , sub_button("-")
            , add_button("+") {
        text_editor.setInputRestrictions(0, "0123456789.-+eE");

        addAndMakeVisible(text_editor);
        addAndMakeVisible(sub_button);
        addAndMakeVisible(add_button);

        set_text(0, false);
    }
    void resized() override {
        auto button_width = 25;
        auto bounds = getLocalBounds();

        sub_button.setBounds(bounds.withWidth(button_width));
        text_editor.setBounds(bounds.reduced(button_width + 2, 0));
        add_button.setBounds(bounds.withLeft(getWidth() - button_width));
    }

    void buttonClicked(Button* button) override {
        if (button == &sub_button) {
            set_value(get_value() - 1, true);
        } else if (button == &add_button) {
            set_value(get_value() + 1, true);
        }
    }

    void textEditorReturnKeyPressed(TextEditor& editor) override {
        if (&editor == &text_editor) {
            set_value(std::stof(editor.getText().toStdString()), true);
        }
    }

    void set_value(T x, bool send_changed) {
        set_text(x, false);
        if (send_changed) {
            listener_list.call(&Listener::number_editor_value_changed, *this);
        }
    }
    T get_value() const {
        return value;
    }

    void addListener(Listener* l) {
        listener_list.add(l);
    }
    void removeListener(Listener* l) {
        listener_list.remove(l);
    }

private:
    void set_text(T x, bool send_changed) {
        value = x;

        std::stringstream ss;
        ss << value;
        text_editor.setText(ss.str(), send_changed);
    }

    TextEditor text_editor;
    model::Connector<TextEditor> text_editor_connector{&text_editor, this};

    TextButton sub_button;
    model::Connector<TextButton> sub_button_connector{&sub_button, this};

    TextButton add_button;
    model::Connector<TextButton> add_button_connector{&add_button, this};

    T value{0};

    ListenerList<Listener> listener_list;
};

template <typename T>
class NumberProperty : public PropertyComponent,
                       public NumberEditor<T>::Listener,
                       public ChangeListener {
public:
    NumberProperty(const String& name, model::ValueWrapper<T>& value)
            : PropertyComponent(name)
            , value(value) {
        changeListenerCallback(&value);

        addAndMakeVisible(editor);
    }
    void refresh() override {
    }

    void number_editor_value_changed(NumberEditor<T>& t) override {
        if (&t == &editor) {
            value.set_value(editor.get_value());
        }
    }

    void changeListenerCallback(ChangeBroadcaster* cb) override {
        if (cb == &value) {
            editor.set_value(value.get_value(), false);
        }
    }

private:
    model::ValueWrapper<T>& value;
    model::ChangeConnector value_connector{&value, this};

    NumberEditor<T> editor;
    model::Connector<NumberEditor<T>> editor_connector{&editor, this};
};

class Vec3fEditor : public Component {
public:
    Vec3fEditor(model::ValueWrapper<Vec3f>& value);
    void resized() override;

private:
    model::ValueWrapper<Vec3f>& value;
    PropertyPanel property_panel;
};

class Vec3fProperty : public PropertyComponent {
public:
    Vec3fProperty(const String& name, model::ValueWrapper<Vec3f>& value);
    void refresh() override;

private:
    Vec3fEditor editor;
};