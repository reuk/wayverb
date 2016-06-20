#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

template <typename T>
T lerp(T i, T in_lo, T in_hi, T out_lo, T out_hi) {
    return ((i - in_lo) * (out_hi - out_lo)) / (in_hi - in_lo) + out_lo;
}

template <typename T>
T lerp(T i, Range<T> in, Range<T> out) {
    return lerp(i, in.getStart(), in.getEnd(), out.getStart(), out.getEnd());
}