#pragma once

#include "FileDropComponent.hpp"

class LoadWindow : public DocumentWindow {
public:
    LoadWindow(String name,
               DocumentWindow::TitleBarButtons buttons,
               const std::string& file_formats,
               ApplicationCommandManager& command_manager)
            : DocumentWindow(name, Colours::lightgrey, buttons) {
        content_component.setSize(600, 400);
        content_component.set_valid_file_formats(file_formats);
        setContentNonOwned(&content_component, true);
        setUsingNativeTitleBar(true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        setResizable(false, false);
        setResizeLimits(400, 300, 100000, 100000);
        setVisible(true);
        addKeyListener(command_manager.getKeyMappings());
    }

    void closeButtonPressed() override {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }

    const FileDropComponent& get_content() const {
        return content_component;
    }

    FileDropComponent& get_content() {
        return content_component;
    }

    void addListener(FileDropComponent::Listener* l) {
        content_component.addListener(l);
    }

    void removeListener(FileDropComponent::Listener* l) {
        content_component.removeListener(l);
    }

private:
    FileDropComponent content_component{"drop a file here, or",
                                        "click to load"};
};