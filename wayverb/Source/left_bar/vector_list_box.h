#pragma once

#include "AngularLookAndFeel.h"

#include "combined/model/vector.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <functional>

namespace left_bar {

template <typename T>
class updatable_component : public Component {
public:
    virtual void update(T) = 0;
};

/// Dead simple list box editor.
/// Listens to a model::vector, and updates if the model changes.
/// Allows the component for each row to be created using a factory callback.
template <typename Model>
class vector_list_box final : public ListBox {
public:
    using model_type = Model;
    using value_type = std::decay_t<decltype(std::declval<Model>().front())>;
    using updatable = updatable_component<value_type>;

    using create_list_item =
            std::function<std::unique_ptr<updatable>(value_type)>;

    vector_list_box(Model& model, create_list_item create_list_item)
            : model_{model, std::move(create_list_item)}
            , connection_{
                      model.connect([this](auto&) { this->updateContent(); })} {
        setModel(&model_);
    }

    using selected_rows_changed = util::event<int>;
    selected_rows_changed::connection connect_selected_rows_changed(
            selected_rows_changed::callback_type callback) {
        return model_.connect_selected_rows_changed(std::move(callback));
    }

private:
    class model final : public ListBoxModel {
    public:
        model(Model& model, create_list_item create_list_item)
                : model_{model}
                , create_list_item_{std::move(create_list_item)} {}

        int getNumRows() override { return model_.size(); }

        void paintListBoxItem(
                int row, Graphics& g, int w, int h, bool selected) override {
            if (selected) {
                g.fillAll(Colours::darkgrey.darker());
            }
        }

        Component* refreshComponentForRow(int row,
                                          bool selected,
                                          Component* existing) override {
            if (row < getNumRows()) {
                const auto shared = model_[row];
                if (updatable* v = dynamic_cast<updatable*>(existing)) {
                    v->update(shared);
                } else {
                    existing = create_list_item_(shared).release();
                    existing->setInterceptsMouseClicks(false, true);
                }
            } else {
                delete existing;
                existing = nullptr;
            }
            return existing;
        }

        void selectedRowsChanged(int last) override {
            selected_rows_changed_(last);
        }

        selected_rows_changed::connection connect_selected_rows_changed(
                selected_rows_changed::callback_type callback) {
            return selected_rows_changed_.connect(std::move(callback));
        }

    private:
        Model& model_;
        selected_rows_changed selected_rows_changed_;

        create_list_item create_list_item_;
    };

    model model_;
    typename Model::scoped_connection connection_;
};

}  // namespace left_bar
