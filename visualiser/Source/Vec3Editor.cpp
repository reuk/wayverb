#include "Vec3Editor.hpp"

NumberEditor::NumberEditor()
        : text_editor()
        , sub_button("-")
        , add_button("+") {
    addAndMakeVisible(text_editor);
    addAndMakeVisible(sub_button);
    addAndMakeVisible(add_button);
}

void NumberEditor::resized() {
    auto button_width = 25;
    auto bounds = getLocalBounds();

    sub_button.setBounds(bounds.withWidth(button_width));
    text_editor.setBounds(bounds.reduced(button_width + 2, 0));
    add_button.setBounds(bounds.withLeft(getWidth() - button_width));
}

NumberProperty::NumberProperty(const String& name)
        : PropertyComponent(name) {
    addAndMakeVisible(editor);
}

void NumberProperty::refresh() {
}

LabelledNumberEditor::LabelledNumberEditor(const String& name)
        : label("", name) {
    label.attachToComponent(&editor, true);
    addAndMakeVisible(label);
    addAndMakeVisible(editor);
}

void LabelledNumberEditor::resized() {
    label.setSize(label_width, getHeight());
    editor.setSize(getWidth() - label_width, getHeight());
    editor.setTopRightPosition(getWidth(), 0);
}

Vec3Editor::Vec3Editor()
        : x("x")
        , y("y")
        , z("z") {
    addAndMakeVisible(x);
    addAndMakeVisible(y);
    addAndMakeVisible(z);
}

void Vec3Editor::resized() {
    auto padding = 2;
    auto w = getWidth();
    auto h = (getHeight() - padding * 2) / 3;

    x.setSize(w, h);
    y.setSize(w, h);
    z.setSize(w, h);

    x.setTopLeftPosition(0, 0 * (h + padding));
    y.setTopLeftPosition(0, 1 * (h + padding));
    z.setTopLeftPosition(0, 2 * (h + padding));
}

Vec3Property::Vec3Property(const String& name)
        : PropertyComponent(name, 79) {
    addAndMakeVisible(editor);
}

void Vec3Property::refresh() {
}