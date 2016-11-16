#include "text_property.h"

namespace left_bar {

text_property::text_property(const String& name)
        : PropertyComponent{name, 25} {
    addAndMakeVisible(editor_);
}

std::string text_property::get() const {
    return editor_.getText().toStdString();
}

void text_property::set(const std::string& s) { editor_.setText(s, false); }

void text_property::refresh() {}

text_property::on_change::connection text_property::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

void text_property::textEditorReturnKeyPressed(TextEditor&) {
    on_change_(*this, get());
}

}  // namespace left_bar
