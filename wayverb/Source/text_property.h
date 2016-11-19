#pragma once

#include "UtilityComponents/connector.h"

#include "utilities/event.h"

#include "../JuceLibraryCode/JuceHeader.h"

class text_property final : public PropertyComponent,
                            public TextEditor::Listener {
public:
    text_property(const String& name);

    std::string get() const;
    void set(const std::string& s);

    void refresh() override;

    using on_change = util::event<text_property&, std::string>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    void textEditorTextChanged(TextEditor&) override;

    TextEditor editor;

private:
    model::Connector<TextEditor> editor_connector_{&editor, this};
    on_change on_change_;
};
