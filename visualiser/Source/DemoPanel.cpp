#include "DemoPanel.hpp"

DemoPanel::DemoPanel() {
    button_map[std::make_unique<TextButton>("echo tunnel")] = FilePackage(
        File("/Users/reuben/dev/waveguide/demo/assets/test_models/"
             "echo_tunnel.obj"),
        File("/Users/reuben/dev/waveguide/demo/assets/materials/bright.json"),
        File("/Users/reuben/dev/waveguide/demo/assets/configs/tunnel.json"));

    button_map[std::make_unique<TextButton>("bedroom")] = FilePackage(
        File("/Users/reuben/dev/waveguide/demo/assets/test_models/bedroom.obj"),
        File("/Users/reuben/dev/waveguide/demo/assets/materials/mat.json"),
        File("/Users/reuben/dev/waveguide/demo/assets/configs/bedroom.json"));

    button_map[std::make_unique<TextButton>("vault")] = FilePackage(
        File("/Users/reuben/dev/waveguide/demo/assets/test_models/vault.obj"),
        File("/Users/reuben/dev/waveguide/demo/assets/materials/vault.json"),
        File("/Users/reuben/dev/waveguide/demo/assets/configs/vault.json"));

    for (const auto& i : button_map) {
        addAndMakeVisible(*i.first);
        i.first->setClickingTogglesState(true);
        i.first->addListener(this);
        i.first->setRadioGroupId(0x100);
    }
}

void DemoPanel::resized() {
    auto w = getWidth() / button_map.size();

    auto counter = 0u;
    for (const auto& i : button_map) {
        i.first->setSize(w, getHeight());
        i.first->setTopLeftPosition(counter * getWidth() / button_map.size(),
                                    0);

        counter += 1;
    }
}

void DemoPanel::buttonClicked(Button* b) {
    for (const auto& i : button_map) {
        if (b->getToggleState() && i.first.get() == b) {
            listener_list.call(&Listener::file_package_loaded, *this, i.second);
        }
    }
}

void DemoPanel::add_listener(Listener& l) {
    listener_list.add(&l);
}

void DemoPanel::remove_listener(Listener& l) {
    listener_list.remove(&l);
}