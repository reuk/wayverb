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

    /// Set worldspace units corresponding to a unit amplitude
    virtual void set_amplitude_scale(float f) = 0;

    /// Set worldspace units corresponding to a single second
    virtual void set_time_scale(float f) = 0;

    /// Set range in seconds that should be shown
    virtual void set_visible_range(const Range<float>& r) = 0;
};