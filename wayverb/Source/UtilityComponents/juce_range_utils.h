#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

template <typename T, typename U, typename V>
auto map(T x, Range<U> from, Range<V> to) {
    return (((x - from.getStart()) * to.getLength()) / from.getLength()) +
           to.getStart();
}

template <typename T>
auto make_range(T a, T b) {
    return Range<T>::between(a, b);
}

