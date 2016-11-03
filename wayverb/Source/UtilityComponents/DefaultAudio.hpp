#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DefaultAudioFormatManager : public juce::AudioFormatManager {
public:
    DefaultAudioFormatManager();
};

class DefaultAudioDeviceManager : public juce::AudioDeviceManager {
public:
    DefaultAudioDeviceManager();
};
