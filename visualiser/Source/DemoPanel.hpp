#pragma once

#include "FilePackage.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

#include <map>

class DemoPanel final : public Component, public Button::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() noexcept = default;
        virtual void file_package_loaded(DemoPanel&, const FilePackage& fp) = 0;
    };

    DemoPanel();
    void resized() override;
    void buttonClicked(Button* b) override;

    void add_listener(Listener& l);
    void remove_listener(Listener& l);

private:
    ListenerList<Listener> listener_list;
    std::map<std::unique_ptr<TextButton>, FilePackage> button_map;
};