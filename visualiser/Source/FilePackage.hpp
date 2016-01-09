#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class FilePackage {
public:
    FilePackage(const File& object = File(),
                const File& material = File(),
                const File& config = File());

    File get_object() const;
    File get_material() const;
    File get_config() const;

private:
    File object;
    File material;
    File config;
};