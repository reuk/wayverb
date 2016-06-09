#pragma once

#include "modern_gl_utils/drawable.h"

#include "../JuceLibraryCode/JuceHeader.h"

class GLAudioThumbnailBase
        : public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver {
public:
    GLAudioThumbnailBase() = default;
    virtual ~GLAudioThumbnailBase() noexcept = default;

    virtual void clear() = 0;
    virtual void load_from(AudioFormatManager&, const File& file) = 0;
};