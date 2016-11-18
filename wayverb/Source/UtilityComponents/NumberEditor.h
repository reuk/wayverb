#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include <sstream>

template <typename T>
class IncDecButtons : public juce::Component, public juce::Button::Listener {
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

        sub_button.addListener(this);
        add_button.addListener(this);

        sub_button.addMouseListener(this, false);
        add_button.addMouseListener(this, false);

        sub_button.setWantsKeyboardFocus(false);
        add_button.setWantsKeyboardFocus(false);
    }

    void resized() override {
        auto width = getWidth();
        auto button_width = (width - 2) / 2;

        sub_button.setSize(button_width, getHeight());
        add_button.setSize(button_width, getHeight());

        sub_button.setTopLeftPosition(0, 0);
        add_button.setTopLeftPosition(sub_button.getRight() + 2, 0);
    }

    void set_increment(T t) { increment = t; }

    T get_increment() const { return increment; }

    void addListener(Listener* l) { listener_list.add(l); }
    void removeListener(Listener* l) { listener_list.remove(l); }

    void mouseDown(const juce::MouseEvent& e) override {
        dragged = false;
        mouse_start_pos = mouse_pos_when_last_dragged = e.position;
        if (isEnabled()) {
            value_on_mouse_down = value_on_prev_frame =
                    value_when_last_dragged = 0;
            mouseDrag(e);
        }
    }

    void handleDrag(const juce::MouseEvent& e) {
        auto mouse_diff = e.position.x - mouse_start_pos.x;

        constexpr auto pixels_for_single_increment = 50;
        value_when_last_dragged =
                increment * mouse_diff / pixels_for_single_increment;

        auto since_last = e.position.x - mouse_pos_when_last_dragged.x;

        add_button.setState(since_last < 0 ? juce::Button::buttonNormal
                                           : juce::Button::buttonOver);
        sub_button.setState(since_last > 0 ? juce::Button::buttonNormal
                                           : juce::Button::buttonOver);

        e.source.enableUnboundedMouseMovement(true, false);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        if (isEnabled()) {
            if (!dragged) {
                if (e.getDistanceFromDragStart() < 10 ||
                    !e.mouseWasDraggedSinceMouseDown()) {
                    return;
                }

                dragged = true;
                mouse_start_pos = e.position;
            }

            handleDrag(e);

            listener_list.call(&Listener::apply_increment,
                               this,
                               value_when_last_dragged - value_on_prev_frame);
            value_on_prev_frame = value_when_last_dragged;

            mouse_pos_when_last_dragged = e.position;
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        if (isEnabled()) {
            add_button.setState(juce::Button::buttonNormal);
            sub_button.setState(juce::Button::buttonNormal);
        }
    }

    void buttonClicked(juce::Button* b) override {
        listener_list.call(&Listener::apply_increment,
                           this,
                           b == &add_button ? increment : -increment);
    }

private:
    juce::TextButton sub_button{"-"};
    juce::TextButton add_button{"+"};

    T increment{1};

    bool dragged{false};
    juce::Point<float> mouse_start_pos;
    juce::Point<float> mouse_pos_when_last_dragged;
    T value_on_mouse_down;
    T value_on_prev_frame;
    T value_when_last_dragged;

    juce::ListenerList<Listener> listener_list;
};

template <typename T>
class NumberEditor : public juce::Component,
                     public juce::TextEditor::Listener,
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

        virtual void number_editor_value_changed(NumberEditor* e) = 0;
    };

    NumberEditor() {
        text_editor.setInputRestrictions(0, "0123456789.-+eE");
        text_editor.setSelectAllWhenFocused(true);
        text_editor.setWantsKeyboardFocus(true);

        addAndMakeVisible(text_editor);
        addAndMakeVisible(inc_dec_buttons);

        text_editor.addListener(this);
        inc_dec_buttons.addListener(this);

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

    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override {
        if (&editor == &text_editor) {
            set_value(get_value(), false);
        }
    }

    void textEditorReturnKeyPressed(juce::TextEditor& editor) override {
        if (&editor == &text_editor) {
            try {
                set_value(std::stof(editor.getText().toStdString()), true);
            } catch (const std::invalid_argument& e) {
                set_value(0, true);
            } catch (std::out_of_range& e) {
                set_value(0, true);
            }
        }
    }

    void textEditorFocusLost(juce::TextEditor& editor) override {
        textEditorReturnKeyPressed(editor);
        editor.setHighlightedRegion(juce::Range<int>(0, 0));
    }

    void set_value(T x, bool send_changed) {
        set_text(x, false);
        if (send_changed) {
            // listener_list.call(&Listener::number_editor_value_changed, this);
        }
    }
    T get_value() const { return value; }

    void set_increment(T t) { inc_dec_buttons.set_increment(t); }

    T get_increment() const { return inc_dec_buttons.get_increment(); }

private:
    void set_text(T x, bool send_changed) {
        value = x;

        std::stringstream ss;
        ss << value;
        text_editor.setText(ss.str(), send_changed);
    }

    juce::TextEditor text_editor;
    IncDecButtons<T> inc_dec_buttons;

    T value{0};
};
