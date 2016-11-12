#include "DefaultAudio.h"

DefaultAudioFormatManager::DefaultAudioFormatManager() {
    registerBasicFormats();
}

DefaultAudioDeviceManager::DefaultAudioDeviceManager() {
    initialiseWithDefaultDevices(0, 2);
}
