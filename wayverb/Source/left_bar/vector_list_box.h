#pragma once

#include "AngularLookAndFeel.h"

#include "combined/model/vector.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <functional>

namespace left_bar {

/// Dead simple list box editor.
/// Listens to a model::vector, and updates if the model changes.
/// Allows the component for each row to be created using a factory callback.
template <typename Model>
class vector_list_box final : public ListBox {
public:
    using model_type = Model;

    using new_component_for_row =
            std::function<std::unique_ptr<Component>(int row, bool selected)>;

    vector_list_box(Model& model, new_component_for_row new_component_for_row)
            : model_{model, std::move(new_component_for_row)} {
        /// If model changes, update the listbox view.
        model.connect([this](auto&) { this->updateContent(); });

        setModel(&model_);
    }

    using selected_rows_changed = util::event<int>;
    selected_rows_changed::connection connect_selected_rows_changed(
        selected_rows_changed::callback_type callback) {
        model_.connect_selected_rows_changed(std::move(callback));
    }

private:
    class model final : public ListBoxModel {
    public:
        model(Model& model, new_component_for_row new_component_for_row)
                : model_{model}
                , new_component_for_row_{std::move(new_component_for_row)} {}

        int getNumRows() override { return model_.size(); }

        void paintListBoxItem(int row, Graphics& g, int w, int h, bool selected) override {
            if (selected) {
                g.fillAll(Colours::darkgrey.darker());
            }
        }

        Component* refreshComponentForRow(int row,
                                          bool selected,
                                          Component* existing) override {
            if (existing) {
                delete existing;
                existing = nullptr;
            }
            if (row < getNumRows()) {
                existing = new_component_for_row_(row, selected).release();
            }
            return existing;
        }

        void selectedRowsChanged(int last) override {
            selected_rows_changed_(last);
        }

        selected_rows_changed::connection connect_selected_rows_changed(
            selected_rows_changed::callback_type callback) {
            selected_rows_changed_.connect(std::move(callback));
        }

    private:
        Model& model_;
        new_component_for_row new_component_for_row_;
        selected_rows_changed selected_rows_changed_;
    };

    model model_;
};

}  // namespace left_bar
