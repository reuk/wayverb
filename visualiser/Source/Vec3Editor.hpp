#pragma once

#include "common/vec.h"

#include "ValueWrapperSlider.hpp"

#include <array>
#include <iomanip>
#include <sstream>

template <typename T>
class IncDecButtons : public Component {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener& rhs) = default;
        Listener& operator=(const Listener& rhs) = default;
        Listener(Listener&& rhs) noexcept = default;
        Listener& operator=(Listener&& rhs) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void apply_increment(IncDecButtons* b, T value) = 0;
    };

    IncDecButtons() {
        addAndMakeVisible(sub_button);
        addAndMakeVisible(add_button);

        sub_button.setInterceptsMouseClicks(false, false);
        add_button.setInterceptsMouseClicks(false, false);
    }

    void resized() override {
        auto width = getWidth();
        auto button_width = (width - 2) / 2;

        sub_button.setSize(button_width, getHeight());
        add_button.setSize(button_width, getHeight());

        sub_button.setTopLeftPosition(0, 0);
        add_button.setTopLeftPosition(sub_button.getRight() + 2, 0);
    }

    void set_increment(T t) {
        increment = t;
    }

    T get_increment() const {
        return increment;
    }

    void addListener(Listener* l) {
        listener_list.add(l);
    }
    void removeListener(Listener* l) {
        listener_list.remove(l);
    }

    void mouseDown(const MouseEvent& e) override {
        dragged = false;
        mouse_start_pos = mouse_pos_when_last_dragged = e.position;
        if (isEnabled()) {
            value_on_mouse_down = value_on_prev_frame =
                value_when_last_dragged = 0;
            mouseDrag(e);
        }
    }

    void handleDrag(const MouseEvent& e) {
        auto mouseDiff = e.position.x - mouse_start_pos.x;

        auto pixelsForFullDragExtent = 250;
        value_when_last_dragged =
            increment * mouseDiff / pixelsForFullDragExtent;

        add_button.setState(mouseDiff < 0 ? Button::buttonNormal
                                          : Button::buttonDown);
        sub_button.setState(mouseDiff > 0 ? Button::buttonNormal
                                          : Button::buttonDown);
    }

    void mouseDrag(const MouseEvent& e) override {
        if (!dragged) {
            if (e.getDistanceFromDragStart() < 10 ||
                !e.mouseWasDraggedSinceMouseDown()) {
                return;
            }

            dragged = true;
            mouse_start_pos = e.position;
        }

        handleDrag(e);

        //  limit value_when_last_dragged here

        listener_list.call(&Listener::apply_increment,
                           this,
                           value_when_last_dragged - value_on_prev_frame);
        value_on_prev_frame = value_when_last_dragged;

        mouse_pos_when_last_dragged = e.position;
    }

    void mouseUp(const MouseEvent& e) override {
        add_button.setState(Button::buttonNormal);
        sub_button.setState(Button::buttonNormal);

        if (!dragged) {
            if (add_button.getBounds().contains(e.getPosition())) {
                listener_list.call(&Listener::apply_increment, this, increment);
            } else if (sub_button.getBounds().contains(e.getPosition())) {
                listener_list.call(
                    &Listener::apply_increment, this, -increment);
            }
        }
    }

private:
    TextButton sub_button{"-"};
    TextButton add_button{"+"};

    T increment{1};

    bool dragged{false};
    Point<float> mouse_start_pos;
    Point<float> mouse_pos_when_last_dragged;
    T value_on_mouse_down;
    T value_on_prev_frame;
    T value_when_last_dragged;

    ListenerList<Listener> listener_list;
};

template <typename T>
class NumberEditor : public Component,
                     public TextEditor::Listener,
                     public IncDecButtons<T>::Listener {
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

    NumberEditor() {
        text_editor.setInputRestrictions(0, "0123456789.-+eE");

        addAndMakeVisible(text_editor);
        addAndMakeVisible(inc_dec_buttons);

        set_text(0, false);
    }

    void resized() override {
        auto button_width = 52;
        auto bounds = getLocalBounds();

        text_editor.setBounds(bounds.withTrimmedRight(button_width + 2));
        inc_dec_buttons.setBounds(bounds.removeFromRight(button_width));
    }

    void apply_increment(IncDecButtons<T>* idb, T value) override {
        if (idb == &inc_dec_buttons) {
            set_value(get_value() + value, true);
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

    void set_increment(T t) {
        inc_dec_buttons.set_increment(t);
    }

    T get_increment() const {
        return inc_dec_buttons.get_increment();
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

    IncDecButtons<T> inc_dec_buttons;
    model::Connector<IncDecButtons<T>> buttons_connector{&inc_dec_buttons,
                                                         this};

    T value{0};

    ListenerList<Listener> listener_list;
};

template <typename T>
class NumberProperty : public PropertyComponent,
                       public NumberEditor<T>::Listener,
                       public model::BroadcastListener {
public:
    NumberProperty(const String& name, model::ValueWrapper<T>& value)
            : PropertyComponent(name)
            , value(value) {
        receive_broadcast(&value);

        addAndMakeVisible(editor);
    }
    void refresh() override {
    }

    void number_editor_value_changed(NumberEditor<T>& t) override {
        if (&t == &editor) {
            value.set(editor.get_value());
        }
    }

    void receive_broadcast(model::Broadcaster* cb) override {
        if (cb == &value) {
            editor.set_value(value, false);
        }
    }

private:
    model::ValueWrapper<T>& value;
    model::BroadcastConnector value_connector{&value, this};

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