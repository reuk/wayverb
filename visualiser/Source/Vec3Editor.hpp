#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <array>

class NumberEditor : public Component {
public:
    NumberEditor();
    void resized() override;

private:
    TextEditor text_editor;
    TextButton sub_button;
    TextButton add_button;
};

class NumberProperty : public PropertyComponent {
public:
    NumberProperty(const String& name);
    void refresh() override;

private:
    NumberEditor editor;
};

class LabelledNumberEditor : public Component {
public:
    LabelledNumberEditor(const String& name);
    void resized() override;

    static const auto label_width = 50;

private:
    Label label;
    NumberEditor editor;
};

class Vec3Editor : public Component {
public:
    Vec3Editor();
    void resized() override;

private:
    LabelledNumberEditor x;
    LabelledNumberEditor y;
    LabelledNumberEditor z;
};

class Vec3Property : public PropertyComponent {
public:
    Vec3Property(const String& name);
    void refresh() override;

private:
    Vec3Editor editor;
};