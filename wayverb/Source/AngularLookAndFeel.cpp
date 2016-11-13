#include "AngularLookAndFeel.h"

const juce::Colour AngularLookAndFeel::emphasis{0xff009900};

juce::Colour AngularLookAndFeel::create_base_colour(
        juce::Colour button_colour,
        bool has_keyboard_focus,
        bool is_mouse_over,
        bool is_button_down) noexcept {
    const float sat = has_keyboard_focus ? 1.3f : 0.9f;
    const juce::Colour base_colour(button_colour.withMultipliedSaturation(sat));

    if (is_button_down) {
        return base_colour.contrasting(0.2f);
    }
    if (is_mouse_over) {
        return base_colour.contrasting(0.1f);
    }

    return base_colour;
}

AngularLookAndFeel::AngularLookAndFeel() {
    setColour(juce::ProgressBar::ColourIds::backgroundColourId,
              juce::Colours::darkgrey);
    setColour(juce::ProgressBar::ColourIds::foregroundColourId, emphasis);
    setColour(juce::TextButton::ColourIds::buttonColourId,
              juce::Colours::darkgrey);
    setColour(juce::TextButton::ColourIds::buttonOnColourId, emphasis);
    setColour(juce::TextButton::ColourIds::textColourOnId,
              juce::Colours::lightgrey);
    setColour(juce::TextButton::ColourIds::textColourOffId,
              juce::Colours::lightgrey);
    setColour(juce::ToggleButton::ColourIds::textColourId,
              juce::Colours::lightgrey);
    setColour(juce::Label::ColourIds::textColourId, juce::Colours::lightgrey);
    setColour(juce::GroupComponent::ColourIds::textColourId,
              juce::Colours::lightgrey);
    setColour(juce::PropertyComponent::ColourIds::backgroundColourId,
              juce::Colours::darkgrey);
    setColour(juce::PropertyComponent::ColourIds::labelTextColourId,
              juce::Colours::lightgrey);
    setColour(juce::BubbleComponent::ColourIds::backgroundColourId,
              juce::Colours::lightgrey.withAlpha(0.8f));
    setColour(juce::BubbleComponent::ColourIds::outlineColourId,
              juce::Colours::black);
    setColour(juce::Slider::ColourIds::thumbColourId, emphasis);
    setColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId,
              juce::Colours::darkgrey);
    setColour(juce::ComboBox::ColourIds::arrowColourId,
              juce::Colours::lightgrey);
    setColour(juce::TextEditor::ColourIds::textColourId, juce::Colours::black);
    setColour(juce::TextEditor::ColourIds::highlightColourId,
              juce::Colours::darkgrey);
    setColour(juce::TextEditor::ColourIds::highlightedTextColourId,
              juce::Colours::white);

    setUsingNativeAlertWindows(true);
}

void horizontal_line(juce::Graphics& g,
                     int y,
                     int left,
                     int right,
                     const juce::Colour& colour = juce::Colours::white) {
    if (left < right - 1) {
        g.setColour(colour.withAlpha(0.2f));
        g.drawHorizontalLine(y, left + 1, right - 1);
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawHorizontalLine(y, left, left + 1);
        g.drawHorizontalLine(y, right - 1, right);
    }
}

void vertical_line(juce::Graphics& g,
                   int x,
                   int top,
                   int bottom,
                   const juce::Colour& colour = juce::Colours::white) {
    if (top < bottom - 1) {
        g.setColour(colour.withAlpha(0.2f));
        g.drawVerticalLine(x, top + 1, bottom - 1);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawVerticalLine(x, top, top + 1);
        g.drawVerticalLine(x, bottom - 1, bottom);
    }
}

auto matte_outer(juce::Graphics& g,
                 juce::Rectangle<int>
                         bounds,
                 bool vertical,
                 juce::Colour c = juce::Colours::black) {
    if (vertical) {
        vertical_line(g, bounds.getX(), bounds.getY(), bounds.getBottom());
        bounds.removeFromLeft(1);
    } else {
        horizontal_line(
                g, bounds.getBottom() - 1, bounds.getX(), bounds.getRight());
        bounds.removeFromBottom(1);
    }
    g.setColour(c.withAlpha(0.9f));
    g.drawRect(bounds.getX(),
               bounds.getY(),
               bounds.getWidth(),
               bounds.getHeight(),
               1);
    bounds.reduce(1, 1);
    return bounds;
}

void matte_inner(juce::Graphics& g,
                 const juce::Rectangle<int>& bounds,
                 bool vertical,
                 const juce::Colour& colour) {
    g.setColour(colour);
    g.fillRect(bounds);

    auto grad = juce::Colours::white.withAlpha(0.2f);

    if (vertical) {
        g.setGradientFill(juce::ColourGradient(juce::Colours::transparentWhite,
                                               bounds.getX(),
                                               bounds.getY(),
                                               grad,
                                               bounds.getRight(),
                                               bounds.getY(),
                                               false));
        g.fillRect(bounds.getX(),
                   bounds.getY(),
                   bounds.getWidth(),
                   bounds.getHeight());
        vertical_line(g,
                      bounds.getWidth() + 1,
                      bounds.getY(),
                      bounds.getHeight() + 1);
    } else {
        g.setGradientFill(juce::ColourGradient(grad,
                                               bounds.getX(),
                                               bounds.getY(),
                                               juce::Colours::transparentWhite,
                                               bounds.getX(),
                                               bounds.getBottom(),
                                               false));
        g.fillRect(bounds.getX(),
                   bounds.getY(),
                   bounds.getWidth(),
                   bounds.getHeight());

        horizontal_line(g, bounds.getY(), bounds.getX(), bounds.getRight());
    }
}

void AngularLookAndFeel::matte_background_box(juce::Graphics& g,
                                              juce::Rectangle<int>
                                                      bounds,
                                              bool vertical,
                                              const juce::Colour& colour) {
    bounds = matte_outer(g, bounds, vertical);
    if (vertical) {
        g.setGradientFill(
                juce::ColourGradient(colour,
                                     bounds.getX(),
                                     0,
                                     juce::Colours::black.withAlpha(0.5f),
                                     bounds.getRight(),
                                     0,
                                     false));
    } else {
        g.setGradientFill(
                juce::ColourGradient(juce::Colours::black.withAlpha(0.5f),
                                     0,
                                     bounds.getY(),
                                     colour,
                                     0,
                                     bounds.getBottom(),
                                     false));
    }
    g.fillRect(bounds.getX(),
               bounds.getY(),
               bounds.getWidth(),
               bounds.getHeight());
}

void AngularLookAndFeel::matte_foreground_box(juce::Graphics& g,
                                              juce::Rectangle<int>
                                                      bounds,
                                              const juce::Colour& colour) {
    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.drawRect(bounds.getX(),
               bounds.getY(),
               bounds.getWidth(),
               bounds.getHeight(),
               1);

    bounds.reduce(1, 1);

    matte_inner(g, bounds, false, colour);
}

void AngularLookAndFeel::matte_box(juce::Graphics& g,
                                   const juce::Rectangle<int>& bounds,
                                   bool vertical,
                                   const juce::Colour& colour) {
    matte_inner(g, matte_outer(g, bounds, vertical), vertical, colour);
}

void AngularLookAndFeel::drawProgressBar(juce::Graphics& g,
                                         juce::ProgressBar& bar,
                                         int width,
                                         int height,
                                         double progress,
                                         const juce::String& textToShow) {
    const juce::Colour background(
            bar.findColour(juce::ProgressBar::backgroundColourId));
    const juce::Colour foreground(
            bar.findColour(juce::ProgressBar::foregroundColourId));

    //  do the backround
    g.fillAll(background);

    matte_background_box(
            g, juce::Rectangle<int>(0, 0, width, height), false, background);

    //  now the bar
    if (0 <= progress && progress < 1) {
        //  solid bar
        matte_foreground_box(
                g,
                juce::Rectangle<int>(0,
                                     0,
                                     juce::jlimit(0.0,
                                                  static_cast<double>(width),
                                                  progress * width),
                                     height - 1),
                foreground);
    } else {
        //  spinny bar
        g.setColour(foreground);

        const int stripeWidth = height * 2;
        const int position =
                (int)(juce::Time::getMillisecondCounter() / 15) % stripeWidth;

        juce::Path p;

        for (float x = (float)(-position); x < width + stripeWidth;
             x += stripeWidth)
            p.addQuadrilateral(x,
                               0.0f,
                               x + stripeWidth * 0.5f,
                               0.0f,
                               x,
                               (float)height,
                               x - stripeWidth * 0.5f,
                               (float)height);

        juce::Image im(juce::Image::ARGB, width, height, true);

        {
            juce::Graphics g2(im);
            matte_foreground_box(g2,
                                 juce::Rectangle<int>(0, 0, width, height - 1),
                                 foreground);
        }

        g.setTiledImageFill(im, 0, 0, 0.85f);
        g.fillPath(p);
    }

    if (textToShow.isNotEmpty()) {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(height * 0.6f);

        g.drawText(textToShow,
                   0,
                   0,
                   width,
                   height,
                   juce::Justification::centred,
                   false);
    }
}

void AngularLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                              juce::Button& button,
                                              const juce::Colour& background,
                                              bool is_mouse_over,
                                              bool is_button_down) {
    const juce::Colour base_colour(
            create_base_colour(background,
                               button.hasKeyboardFocus(true),
                               is_mouse_over,
                               is_button_down)
                    .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
    matte_box(g,
              juce::Rectangle<int>(0, 0, button.getWidth(), button.getHeight()),
              false,
              base_colour);
}

void AngularLookAndFeel::drawStretchableLayoutResizerBar(
        juce::Graphics& g,
        int w,
        int h,
        bool is_vertical,
        bool is_mouse_over,
        bool is_mouse_dragging) {
    g.fillAll(create_base_colour(
            juce::Colours::white, false, is_mouse_over, is_mouse_dragging));
}

void AngularLookAndFeel::fillTextEditorBackground(juce::Graphics& g,
                                                  int width,
                                                  int height,
                                                  juce::TextEditor&) {
    g.setColour(juce::Colours::white);
    g.fillRect(1, 1, width - 1, height - 2);
}

void AngularLookAndFeel::drawTextEditorOutline(juce::Graphics& g,
                                               int width,
                                               int height,
                                               juce::TextEditor& e) {
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(0, 0, width, height);
    matte_outer(g,
                juce::Rectangle<int>(0, 0, width, height),
                false,
                e.hasKeyboardFocus(true) ? emphasis : juce::Colours::black);
}

void AngularLookAndFeel::drawGroupComponentOutline(
        juce::Graphics& g,
        int width,
        int height,
        const juce::String& text,
        const juce::Justification& position,
        juce::GroupComponent& group) {
    const auto text_h = 15.0f;
    const auto indent = 3.0f;
    const auto text_edge_gap = 4.0f;

    juce::Font f(text_h);

    auto x = indent;
    auto y = f.getAscent() - 3.0f;
    auto w = fmax(0.0f, width - x * 2.0f);
    auto h = fmax(0.0f, height - y - indent);

    float text_w =
            text.isEmpty()
                    ? 0
                    : juce::jlimit(0.0,
                                   fmax(0.0, w - text_edge_gap * 2.0),
                                   f.getStringWidth(text) +
                                                      text_edge_gap * 2.0);
    float text_x = text_edge_gap;

    if (position.testFlags(juce::Justification::horizontallyCentred))
        text_x = (w - text_w) * 0.5f;
    else if (position.testFlags(juce::Justification::right))
        text_x = w - text_w - text_edge_gap;

    horizontal_line(g, y + 1, x + text_x + text_w, x + w);
    horizontal_line(g, y + h + 1, x, x + w);
    horizontal_line(g, y + 1, x, x + text_x);

    g.setColour(juce::Colours::black.withAlpha(0.9f));

    g.drawVerticalLine(x, y, y + h);
    g.drawVerticalLine(x + w, y, y + h);

    g.drawHorizontalLine(y, x + text_x + text_w, x + w);
    g.drawHorizontalLine(y + h, x, x + w);
    g.drawHorizontalLine(y, x, x + text_x);

    const float alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColour(group.findColour(juce::GroupComponent::outlineColourId)
                        .withMultipliedAlpha(alpha));

    g.setColour(group.findColour(juce::GroupComponent::textColourId)
                        .withMultipliedAlpha(alpha));
    g.setFont(f);
    g.drawText(text,
               juce::roundToInt(x + text_x),
               0,
               juce::roundToInt(text_w),
               juce::roundToInt(text_h),
               juce::Justification::centred,
               true);
}

void AngularLookAndFeel::drawPropertyPanelSectionHeader(
        juce::Graphics& g,
        const juce::String& name,
        bool is_open,
        int width,
        int height) {
    //    auto background =
    //        findColour(PropertyComponent::ColourIds::backgroundColourId);

    matte_box(g, juce::Rectangle<int>(0, 0, width, height), false, emphasis);

    auto button_size = height * 0.75f;
    auto button_indent = (height - button_size) * 0.5f;

    drawTreeviewPlusMinusBox(
            g,
            juce::Rectangle<float>(
                    button_indent, button_indent, button_size, button_size),
            juce::Colours::white,
            is_open,
            false);

    auto text_x = (int)(button_indent * 2.0f + button_size + 2.0f);

    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(height * 0.7f, juce::Font::bold));
    g.drawText(name,
               text_x,
               0,
               width - text_x - 4,
               height,
               juce::Justification::centredLeft,
               true);
}

void AngularLookAndFeel::drawPropertyComponentBackground(
        juce::Graphics& g, int width, int height, juce::PropertyComponent& p) {
}

void AngularLookAndFeel::drawLinearSliderBackground(
        juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float slider_pos,
        float min_slider_pos,
        float max_slider_pos,
        const juce::Slider::SliderStyle style,
        juce::Slider&) {
    if (style == juce::Slider::SliderStyle::LinearVertical) {
        juce::Rectangle<int> bounds(x, y, width, height);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(
                bounds.withSizeKeepingCentre(3, height).toFloat(), 2);
    }
}

void AngularLookAndFeel::drawLinearSliderThumb(
        juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float slider_pos,
        float min_slider_pos,
        float max_slider_pos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) {
    juce::Colour thumb_colour = create_base_colour(
            slider.findColour(juce::Slider::thumbColourId),
            slider.hasKeyboardFocus(false) && slider.isEnabled(),
            slider.isMouseOverOrDragging() && slider.isEnabled(),
            slider.isMouseButtonDown() && slider.isEnabled());
    if (style == juce::Slider::SliderStyle::LinearVertical ||
        style == juce::Slider::SliderStyle::LinearHorizontal) {
        const auto slider_radius = (float)(getSliderThumbRadius(slider));

        if (style == juce::Slider::LinearVertical) {
            matte_foreground_box(
                    g,
                    juce::Rectangle<int>(x + width * 0.5 - slider_radius,
                                         slider_pos - slider_radius + 3,
                                         slider_radius * 2,
                                         slider_radius * 2 - 6),
                    thumb_colour);
        } else {
            //            auto kx = slider_pos;
            //            auto ky = y + height * 0.5f;
        }
    }
}

void AngularLookAndFeel::drawComboBox(juce::Graphics& g,
                                      int width,
                                      int height,
                                      bool button_down,
                                      int button_x,
                                      int button_y,
                                      int button_w,
                                      int button_h,
                                      juce::ComboBox& cb) {
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(0, 0, width, height);
    matte_outer(g, juce::Rectangle<int>(0, 0, width, height), false);

    matte_foreground_box(
            g,
            juce::Rectangle<int>(button_x, button_y, button_w, button_h - 1),
            create_base_colour(juce::Colours::darkgrey,
                               cb.hasKeyboardFocus(true),
                               cb.isMouseOver(),
                               button_down));

    const float arrowX = 0.3f;
    const float arrowH = 0.2f;

    juce::Path p;
    p.addTriangle(button_x + button_w * 0.5f,
                  button_y + button_h * (0.45f - arrowH),
                  button_x + button_w * (1.0f - arrowX),
                  button_y + button_h * 0.45f,
                  button_x + button_w * arrowX,
                  button_y + button_h * 0.45f);

    p.addTriangle(button_x + button_w * 0.5f,
                  button_y + button_h * (0.55f + arrowH),
                  button_x + button_w * (1.0f - arrowX),
                  button_y + button_h * 0.55f,
                  button_x + button_w * arrowX,
                  button_y + button_h * 0.55f);

    g.setColour(cb.findColour(juce::ComboBox::arrowColourId)
                        .withMultipliedAlpha(cb.isEnabled() ? 1.0f : 0.3f));
    g.fillPath(p);
}

void AngularLookAndFeel::drawCallOutBoxBackground(juce::CallOutBox& box,
                                                  juce::Graphics& g,
                                                  const juce::Path& path,
                                                  juce::Image& cachedImage) {
    if (cachedImage.isNull()) {
        cachedImage = juce::Image(
                juce::Image::ARGB, box.getWidth(), box.getHeight(), true);
        juce::Graphics g2(cachedImage);

        juce::DropShadow(
                juce::Colours::black.withAlpha(0.7f), 8, juce::Point<int>(0, 2))
                .drawForPath(g2, path);
    }

    g.setColour(juce::Colours::black);
    g.drawImageAt(cachedImage, 0, 0);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.9f));
    g.fillPath(path);

    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void AngularLookAndFeel::drawTickBox(juce::Graphics& g,
                                     juce::Component& component,
                                     float x,
                                     float y,
                                     float w,
                                     float h,
                                     const bool ticked,
                                     const bool isEnabled,
                                     const bool isMouseOverButton,
                                     const bool isButtonDown) {
    const float boxSize = w * 0.7f;

    if (ticked) {
        matte_box(g,
                  juce::Rectangle<int>(
                          x, y + (h - boxSize) * 0.5f, boxSize, boxSize),
                  false,
                  emphasis);
    } else {
        matte_background_box(
                g,
                juce::Rectangle<int>(
                        x, y + (h - boxSize) * 0.5f, boxSize, boxSize),
                false,
                juce::Colours::darkgrey);
    }
}
