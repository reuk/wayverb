#include "PropertyComponentLAF.hpp"

Rectangle<int> PropertyComponentLAF::getPropertyComponentContentPosition(
    PropertyComponent& component) {
    const int textW = jmin(200.0, component.getWidth() * 0.9);
    return Rectangle<int>(
        textW, 1, component.getWidth() - textW - 1, component.getHeight() - 3);
}
