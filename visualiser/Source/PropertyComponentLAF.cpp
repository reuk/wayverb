#include "PropertyComponentLAF.hpp"

PropertyComponentLAF::PropertyComponentLAF(int label_width)
        : label_width(label_width) {
}

Rectangle<int> PropertyComponentLAF::getPropertyComponentContentPosition(
    PropertyComponent& component) {
    const int textW =
        jmin(label_width, static_cast<int>(component.getWidth() * 0.9));
    return Rectangle<int>(
        textW, 1, component.getWidth() - textW - 1, component.getHeight() - 3);
}
