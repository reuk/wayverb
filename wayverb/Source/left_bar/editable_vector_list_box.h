#pragma once

#include "vector_list_box.h"

#include "../UtilityComponents/connector.h"

namespace left_bar {

/// Wraps a list box, giving it add and remove buttons.

template <typename Model>
class editable_vector_list_box final : public Component,
                                       public TextButton::Listener {
public:
    using insert = std::function<void(Model& model)>;

    editable_vector_list_box(
            Model& model,
            typename vector_list_box<Model>::create_list_item create_list_item,
            insert insert)
            : model_{model}
            , connection_{model_.connect(
                      [this](auto&) { this->update_buttons(); })}
            , insert_{std::move(insert)}
            , list_box_{model_, std::move(create_list_item)} {
        //  If selected rows change, update buttons.
        list_box_.connect_selected_rows_changed(
                [this](auto) { update_buttons(); });

        addAndMakeVisible(list_box_);
        addAndMakeVisible(add_button_);
        addAndMakeVisible(sub_button_);
    }

    void resized() override {
        auto bounds = getLocalBounds();

        const auto button_area = bounds.removeFromBottom(25).reduced(0, 2);
        list_box_.setBounds(bounds);
        sub_button_.setBounds(button_area.withTrimmedRight(getWidth() / 2 + 1));
        add_button_.setBounds(button_area.withTrimmedLeft(getWidth() / 2 + 1));
    }

    void buttonClicked(Button* b) override {
        if (b == &add_button_) {
            insert_(model_);
        } else if (b == &sub_button_) {
            const auto to_delete = list_box_.getSelectedRow();
            if (0 <= to_delete && to_delete < model_.size()) {
                list_box_.deselectAllRows();
                model_.erase(model_.begin() + to_delete);
            }
        }
    }

    void setRowHeight(int height) { list_box_.setRowHeight(height); }

private:
    void update_buttons() {
        sub_button_.setEnabled(model_.can_erase() &&
                               list_box_.getSelectedRow() != -1);
    }

    Model& model_;
    typename Model::scoped_connection connection_;

    insert insert_;

    vector_list_box<Model> list_box_;

    TextButton add_button_{"+"};
    model::Connector<TextButton> add_button_connector_{&add_button_, this};

    TextButton sub_button_{"-"};
    model::Connector<TextButton> sub_button_connector_{&sub_button_, this};
};

}  // namespace left_bar
