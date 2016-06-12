#include "DefaultAudio.hpp"

DefaultAudioFormatManager::DefaultAudioFormatManager() {
    registerBasicFormats();
}

DefaultAudioDeviceManager::DefaultAudioDeviceManager() {
    initialiseWithDefaultDevices(0, 2);
}
