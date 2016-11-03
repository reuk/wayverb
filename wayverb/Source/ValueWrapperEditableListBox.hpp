#pragma once

#include "ValueWrapperListBox.hpp"

template <typename T>
class ValueWrapperEditableListBox : public Component,
                                    public T::Listener,
                                    public TextButton::Listener,
                                    public model::BroadcastListener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener& rhs) = default;
        Listener& operator=(const Listener& rhs) = default;
        Listener(Listener&& rhs) noexcept = default;
        Listener& operator=(Listener&& rhs) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void selectedRowsChanged(ValueWrapperEditableListBox* lb,
                                         int last) = 0;
    };

    using value_type = typename T::value_type;
    using model_type = typename T::model_type;

    ValueWrapperEditableListBox(model_type& model)
            : model(model)
            , list_box(model) {
        addAndMakeVisible(list_box);
        addAndMakeVisible(add_button);
        addAndMakeVisible(sub_button);
    }

    void resized() override {
        const auto button_height = 25;
        auto bounds = getLocalBounds();

        list_box.setBounds(bounds.withTrimmedBottom(button_height));

        const auto button_area =
                bounds.removeFromBottom(button_height).reduced(0, 2);
        sub_button.setBounds(button_area.withTrimmedRight(getWidth() / 2 + 1));
        add_button.setBounds(button_area.withTrimmedLeft(getWidth() / 2 + 1));
    }

    void buttonClicked(Button* b) override {
        if (b == &add_button) {
            model.push_back(value_type{});
            list_box.selectRow(model.size() - 1);
        } else if (b == &sub_button) {
            const auto to_delete = list_box.getSelectedRow();
            assert(0 <= to_delete);

            //  deselect first to remove panel listeners/callbacks
            list_box.deselectAllRows();
            //  now delete the microphone
            model.erase(to_delete);
            //  now select the next mic in the list
            list_box.selectRow(std::max(0, to_delete - 1));
        }
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &model) {
            update_sub_button_enablement();
        }
    }

    void selectRow(int row) {
        list_box.selectRow(row);
    }

    void selectedRowsChanged(ValueWrapperListBox<value_type>* b,
                             int last) override {
        listener_list.call(&Listener::selectedRowsChanged, this, last);
        update_sub_button_enablement();
    }

    void addListener(Listener* l) {
        listener_list.add(l);
    }

    void removeListener(Listener* l) {
        listener_list.remove(l);
    }

private:
    void update_sub_button_enablement() {
        const auto more_than_one_entry = 1 < model.size();
        const auto row_selected = list_box.getSelectedRow() != -1;
        sub_button.setEnabled(more_than_one_entry && row_selected);
    }

    model_type& model;
    model::BroadcastConnector model_connector{&model, this};

    ListenerList<Listener> listener_list;

    T list_box;
    model::Connector<T> list_box_connector{&list_box, this};

    TextButton add_button{"+"};
    model::Connector<TextButton> add_button_connector{&add_button, this};

    TextButton sub_button{"-"};
    model::Connector<TextButton> sub_button_connector{&sub_button, this};
};

//----------------------------------------------------------------------------//

template <typename T>
class ListEditorPanel : public Component, public T::Listener {
public:
    using value_type = typename T::value_type;
    using model_type = typename T::model_type;

    ListEditorPanel(model_type& model)
            : model(model)
            , list_box(model) {
        addAndMakeVisible(list_box);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        list_box.setBounds(bounds.removeFromLeft(150));
        if (editor) {
            editor->setBounds(bounds);
        }
    }

    void selectedRowsChanged(T* lb, int last) override {
        assert(last < static_cast<int>(model.size()));
        if (0 <= last) {
            editor = new_editor(model[last]);
            addAndMakeVisible(*editor);
            resized();
        } else {
            editor = nullptr;
        }
    }

private:
    virtual std::unique_ptr<Component> new_editor(
            model::ValueWrapper<value_type>& v) = 0;

    model_type& model;

    T list_box;
    model::Connector<T> list_box_connector{&list_box, this};

    std::unique_ptr<Component> editor;
};
