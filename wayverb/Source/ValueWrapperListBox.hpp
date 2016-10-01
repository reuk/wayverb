#pragma once

#include "DataBinding/collection.hpp"

template <typename T>
class ValueWrapperListBox : public ListBox,
                            public ListBoxModel,
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

        virtual void selectedRowsChanged(ValueWrapperListBox* lb, int last) = 0;
    };

    using value_type = T;
    using model_type = model::ValueWrapper<aligned::vector<value_type>>;

    ValueWrapperListBox(model_type& model)
            : model(model) {
        setModel(this);
    }

    int getNumRows() override {
        return model.size();
    }

    void paintListBoxItem(
            int row, Graphics& g, int w, int h, bool selected) override {
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &model) {
            updateContent();
        }
    }

    void selectedRowsChanged(int last) override {
        listener_list.call(&Listener::selectedRowsChanged, this, last);
    }

    void addListener(Listener* l) {
        listener_list.add(l);
    }
    void removeListener(Listener* l) {
        listener_list.remove(l);
    }

    int get_full_content_height() const {
        return getRowHeight() * model.size();
    }

    model_type& get_model() {
        return model;
    }

    const model_type& get_model() const {
        return model;
    }

    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override {
        if (existing) {
            delete existing;
            existing = nullptr;
        }
        if (row < getNumRows()) {
            existing = new_component_for_row(row, selected).release();
        }
        return existing;
    }

protected:
    static void configure_selectable_label(Label& label, bool selected) {
        label.setColour(Label::ColourIds::textColourId, Colours::black);
        label.setColour(Label::ColourIds::textColourId,
                        selected ? Colours::lightgrey : Colours::black);
        label.setColour(
                Label::ColourIds::backgroundColourId,
                selected ? Colours::darkgrey : Colours::transparentWhite);
        label.setInterceptsMouseClicks(false, false);
    }

private:
    virtual std::unique_ptr<Component> new_component_for_row(int row,
                                                             bool selected) = 0;

    model_type& model;
    model::BroadcastConnector model_connector{&model, this};

    ListenerList<Listener> listener_list;
};
