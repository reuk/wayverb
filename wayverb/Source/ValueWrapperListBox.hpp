#pragma once

#include "FullModel.hpp"

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

    using model_type = model::ValueWrapper<std::vector<T>>;

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

protected:
    model_type& model;
    model::BroadcastConnector model_connector{&model, this};

private:
    ListenerList<Listener> listener_list;
};
