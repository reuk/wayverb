#include "text_property.h"

text_property::text_property(const String& name)
        : PropertyComponent{name, 25} {
    addAndMakeVisible(editor);
}

std::string text_property::get() const {
    return editor.getText().toStdString();
}

void text_property::set(const std::string& s) { editor.setText(s, false); }

void text_property::refresh() {}

text_property::on_change::connection text_property::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

void text_property::textEditorTextChanged(TextEditor&) {
    on_change_(*this, get());
}
