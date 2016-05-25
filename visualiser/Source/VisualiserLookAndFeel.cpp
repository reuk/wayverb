#include "VisualiserLookAndFeel.hpp"

static const Colour emphasis = Colours::blueviolet;

Colour VisualiserLookAndFeel::create_base_colour(Colour button_colour,
                                                 bool has_keyboard_focus,
                                                 bool is_mouse_over,
                                                 bool is_button_down) noexcept {
    const float sat = has_keyboard_focus ? 1.3f : 0.9f;
    const Colour base_colour(button_colour.withMultipliedSaturation(sat));

    if (is_button_down) {
        return base_colour.contrasting(0.2f);
    }
    if (is_mouse_over) {
        return base_colour.contrasting(0.1f);
    }

    return base_colour;
}

VisualiserLookAndFeel::VisualiserLookAndFeel() {
    setColour(ProgressBar::ColourIds::backgroundColourId, Colours::darkgrey);
    setColour(ProgressBar::ColourIds::foregroundColourId, emphasis);
    setColour(TextButton::ColourIds::buttonColourId, Colours::darkgrey);
    setColour(TextButton::ColourIds::textColourOnId, Colours::lightgrey);
    setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    setColour(Label::ColourIds::textColourId, Colours::lightgrey);
    setColour(GroupComponent::ColourIds::textColourId, Colours::lightgrey);
    setColour(PropertyComponent::ColourIds::backgroundColourId,
              Colours::darkgrey);
    setColour(PropertyComponent::ColourIds::labelTextColourId,
              Colours::lightgrey);
    setColour(BubbleComponent::ColourIds::backgroundColourId,
              Colours::lightgrey.withAlpha(0.8f));
    setColour(BubbleComponent::ColourIds::outlineColourId, Colours::black);
    setColour(Slider::ColourIds::thumbColourId, emphasis);
    setColour(PopupMenu::ColourIds::highlightedBackgroundColourId,
              Colours::darkgrey);
    setColour(ComboBox::ColourIds::arrowColourId, Colours::lightgrey);
    setColour(TextEditor::ColourIds::textColourId, Colours::black);
    setColour(TextEditor::ColourIds::highlightColourId, Colours::darkgrey);
    setColour(TextEditor::ColourIds::highlightedTextColourId, Colours::white);
}

void horizontal_line(Graphics& g,
                     int y,
                     int left,
                     int right,
                     const Colour& colour = Colours::white) {
    if (left < right - 1) {
        g.setColour(colour.withAlpha(0.2f));
        g.drawHorizontalLine(y, left + 1, right - 1);
        g.setColour(Colours::white.withAlpha(0.1f));
        g.drawHorizontalLine(y, left, left + 1);
        g.drawHorizontalLine(y, right - 1, right);
    }
}

void vertical_line(Graphics& g,
                   int x,
                   int top,
                   int bottom,
                   const Colour& colour = Colours::white) {
    if (top < bottom - 1) {
        g.setColour(colour.withAlpha(0.2f));
        g.drawVerticalLine(x, top + 1, bottom - 1);
        g.setColour(Colours::white.withAlpha(0.2f));
        g.drawVerticalLine(x, top, top + 1);
        g.drawVerticalLine(x, bottom - 1, bottom);
    }
}

auto matte_outer(Graphics& g,
                 Rectangle<int> bounds,
                 bool vertical,
                 Colour c = Colours::black) {
    if (vertical) {
        vertical_line(g, bounds.getX(), bounds.getY(), bounds.getBottom());
        bounds.removeFromLeft(1);
    } else {
        horizontal_line(
            g, bounds.getHeight() - 1, bounds.getX(), bounds.getRight());
        bounds.removeFromBottom(1);
    }
    g.setColour(c.withAlpha(0.9f));
    g.drawRect(
        bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 1);
    bounds.reduce(1, 1);
    return bounds;
}

void matte_inner(Graphics& g,
                 const Rectangle<int>& bounds,
                 bool vertical,
                 const Colour& colour) {
    g.setColour(colour);
    g.fillRect(bounds);

    auto grad = Colours::white.withAlpha(0.2f);

    if (vertical) {
        g.setGradientFill(ColourGradient(Colours::transparentWhite,
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
        vertical_line(
            g, bounds.getWidth() + 1, bounds.getY(), bounds.getHeight() + 1);
    } else {
        g.setGradientFill(ColourGradient(grad,
                                         bounds.getX(),
                                         bounds.getY(),
                                         Colours::transparentWhite,
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

void matte_background_box(Graphics& g,
                          Rectangle<int> bounds,
                          bool vertical,
                          const Colour& colour) {
    bounds = matte_outer(g, bounds, vertical);
    if (vertical) {
        g.setGradientFill(ColourGradient(colour,
                                         0,
                                         0,
                                         Colours::black.withAlpha(0.5f),
                                         bounds.getWidth(),
                                         0,
                                         false));
    } else {
        g.setGradientFill(ColourGradient(Colours::black.withAlpha(0.5f),
                                         0,
                                         0,
                                         colour,
                                         0,
                                         bounds.getHeight(),
                                         false));
    }
    g.fillRect(
        bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
}

void matte_foreground_box(
    Graphics& g, int x, int y, int width, int height, const Colour& colour) {
    Rectangle<int> bounds(x, y, width, height);

    g.setColour(Colours::black.withAlpha(0.9f));
    g.drawRect(
        bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 1);

    bounds.reduce(1, 1);

    matte_inner(g, bounds, false, colour);
}

void matte_box(Graphics& g,
               const Rectangle<int>& bounds,
               bool vertical,
               const Colour& colour) {
    matte_inner(g, matte_outer(g, bounds, vertical), vertical, colour);
}

void VisualiserLookAndFeel::drawProgressBar(Graphics& g,
                                            ProgressBar& bar,
                                            int width,
                                            int height,
                                            double progress,
                                            const String& textToShow) {
    const Colour background(bar.findColour(ProgressBar::backgroundColourId));
    const Colour foreground(bar.findColour(ProgressBar::foregroundColourId));

    //  do the backround
    g.fillAll(background);

    matte_background_box(
        g, Rectangle<int>(0, 0, width, height), false, background);

    //  now the bar
    if (0 <= progress && progress < 1) {
        //  solid bar
        matte_foreground_box(
            g,
            0,
            0,
            jlimit(0.0, static_cast<double>(width), progress * width),
            height - 1,
            foreground);
    } else {
        //  spinny bar
        g.setColour(foreground);

        const int stripeWidth = height * 2;
        const int position =
            (int)(Time::getMillisecondCounter() / 15) % stripeWidth;

        Path p;

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

        Image im(Image::ARGB, width, height, true);

        {
            Graphics g2(im);
            matte_foreground_box(g2, 0, 0, width, height - 1, foreground);
        }

        g.setTiledImageFill(im, 0, 0, 0.85f);
        g.fillPath(p);
    }

    if (textToShow.isNotEmpty()) {
        g.setColour(Colours::lightgrey);
        g.setFont(height * 0.6f);

        g.drawText(
            textToShow, 0, 0, width, height, Justification::centred, false);
    }
}

void VisualiserLookAndFeel::drawButtonBackground(Graphics& g,
                                                 Button& button,
                                                 const Colour& background,
                                                 bool is_mouse_over,
                                                 bool is_button_down) {
    const Colour base_colour(
        create_base_colour(background,
                           button.hasKeyboardFocus(true),
                           is_mouse_over,
                           is_button_down)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
    matte_box(g,
              Rectangle<int>(0, 0, button.getWidth(), button.getHeight()),
              false,
              base_colour);
}

void VisualiserLookAndFeel::drawStretchableLayoutResizerBar(
    Graphics& g,
    int w,
    int h,
    bool is_vertical,
    bool is_mouse_over,
    bool is_mouse_dragging) {
    g.fillAll(create_base_colour(
        Colours::white, false, is_mouse_over, is_mouse_dragging));
}

void VisualiserLookAndFeel::fillTextEditorBackground(Graphics& g,
                                                     int width,
                                                     int height,
                                                     TextEditor&) {
    g.setColour(Colours::white);
    g.fillRect(1, 1, width - 1, height - 2);
}

void VisualiserLookAndFeel::drawTextEditorOutline(Graphics& g,
                                                  int width,
                                                  int height,
                                                  TextEditor& e) {
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, width, height);
    matte_outer(g,
                Rectangle<int>(0, 0, width, height),
                false,
                e.hasKeyboardFocus(true) ? emphasis : Colours::black);
}

void VisualiserLookAndFeel::drawGroupComponentOutline(
    Graphics& g,
    int width,
    int height,
    const String& text,
    const Justification& position,
    GroupComponent& group) {
    const auto text_h = 15.0f;
    const auto indent = 3.0f;
    const auto text_edge_gap = 4.0f;

    Font f(text_h);

    auto x = indent;
    auto y = f.getAscent() - 3.0f;
    auto w = jmax(0.0f, width - x * 2.0f);
    auto h = jmax(0.0f, height - y - indent);

    float text_w = text.isEmpty()
                       ? 0
                       : jlimit(0.0f,
                                jmax(0.0f, w - text_edge_gap * 2),
                                f.getStringWidth(text) + text_edge_gap * 2.0f);
    float text_x = text_edge_gap;

    if (position.testFlags(Justification::horizontallyCentred))
        text_x = (w - text_w) * 0.5f;
    else if (position.testFlags(Justification::right))
        text_x = w - text_w - text_edge_gap;

    horizontal_line(g, y + 1, x + text_x + text_w, x + w);
    horizontal_line(g, y + h + 1, x, x + w);
    horizontal_line(g, y + 1, x, x + text_x);

    g.setColour(Colours::black.withAlpha(0.9f));

    g.drawVerticalLine(x, y, y + h);
    g.drawVerticalLine(x + w, y, y + h);

    g.drawHorizontalLine(y, x + text_x + text_w, x + w);
    g.drawHorizontalLine(y + h, x, x + w);
    g.drawHorizontalLine(y, x, x + text_x);

    const float alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColour(group.findColour(GroupComponent::outlineColourId)
                    .withMultipliedAlpha(alpha));

    g.setColour(group.findColour(GroupComponent::textColourId)
                    .withMultipliedAlpha(alpha));
    g.setFont(f);
    g.drawText(text,
               roundToInt(x + text_x),
               0,
               roundToInt(text_w),
               roundToInt(text_h),
               Justification::centred,
               true);
}

void VisualiserLookAndFeel::drawPropertyPanelSectionHeader(
    Graphics& g, const String& name, bool is_open, int width, int height) {
    //    auto background =
    //        findColour(PropertyComponent::ColourIds::backgroundColourId);

    matte_box(g, Rectangle<int>(0, 0, width, height), false, emphasis);

    auto button_size = height * 0.75f;
    auto button_indent = (height - button_size) * 0.5f;

    drawTreeviewPlusMinusBox(
        g,
        Rectangle<float>(
            button_indent, button_indent, button_size, button_size),
        Colours::white,
        is_open,
        false);

    auto text_x = (int)(button_indent * 2.0f + button_size + 2.0f);

    g.setColour(Colours::lightgrey);
    g.setFont(Font(height * 0.7f, Font::bold));
    g.drawText(name,
               text_x,
               0,
               width - text_x - 4,
               height,
               Justification::centredLeft,
               true);
}

void VisualiserLookAndFeel::drawPropertyComponentBackground(
    Graphics& g, int width, int height, PropertyComponent& p) {
}

void VisualiserLookAndFeel::drawLinearSliderBackground(
    Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float slider_pos,
    float min_slider_pos,
    float max_slider_pos,
    const Slider::SliderStyle style,
    Slider&) {
    if (style == Slider::SliderStyle::LinearVertical) {
        Rectangle<int> bounds(x, y, width, height);
        g.setColour(Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(
            bounds.withSizeKeepingCentre(3, height).toFloat(), 2);
    }
}

void VisualiserLookAndFeel::drawLinearSliderThumb(
    Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float slider_pos,
    float min_slider_pos,
    float max_slider_pos,
    const Slider::SliderStyle style,
    Slider& slider) {
    Colour thumb_colour =
        create_base_colour(slider.findColour(Slider::thumbColourId),
                           slider.hasKeyboardFocus(false) && slider.isEnabled(),
                           slider.isMouseOverOrDragging() && slider.isEnabled(),
                           slider.isMouseButtonDown() && slider.isEnabled());
    if (style == Slider::SliderStyle::LinearVertical ||
        style == Slider::SliderStyle::LinearHorizontal) {
        const auto slider_radius = (float)(getSliderThumbRadius(slider));

        if (style == Slider::LinearVertical) {
            matte_foreground_box(g,
                                 x + width * 0.5 - slider_radius,
                                 slider_pos - slider_radius + 3,
                                 slider_radius * 2,
                                 slider_radius * 2 - 6,
                                 thumb_colour);
        } else {
            //            auto kx = slider_pos;
            //            auto ky = y + height * 0.5f;
        }
    }
}

void VisualiserLookAndFeel::drawComboBox(Graphics& g,
                                         int width,
                                         int height,
                                         bool button_down,
                                         int button_x,
                                         int button_y,
                                         int button_w,
                                         int button_h,
                                         ComboBox& cb) {
    g.fillAll(Colours::white);
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, width, height);
    matte_outer(g, Rectangle<int>(0, 0, width, height), false);

    matte_foreground_box(g,
                         button_x,
                         button_y,
                         button_w,
                         button_h - 1,
                         create_base_colour(Colours::darkgrey,
                                            cb.hasKeyboardFocus(true),
                                            cb.isMouseOver(),
                                            button_down));

    const float arrowX = 0.3f;
    const float arrowH = 0.2f;

    Path p;
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

    g.setColour(cb.findColour(ComboBox::arrowColourId)
                    .withMultipliedAlpha(cb.isEnabled() ? 1.0f : 0.3f));
    g.fillPath(p);
}

void VisualiserLookAndFeel::drawCallOutBoxBackground(CallOutBox& box,
                                                     Graphics& g,
                                                     const Path& path,
                                                     Image& cachedImage) {
    if (cachedImage.isNull()) {
        cachedImage = Image(Image::ARGB, box.getWidth(), box.getHeight(), true);
        Graphics g2(cachedImage);

        DropShadow(Colours::black.withAlpha(0.7f), 8, Point<int>(0, 2))
            .drawForPath(g2, path);
    }

    g.setColour(Colours::black);
    g.drawImageAt(cachedImage, 0, 0);

    g.setColour(Colours::darkgrey.withAlpha(0.9f));
    g.fillPath(path);

    g.setColour(Colours::white.withAlpha(0.8f));
    g.strokePath(path, PathStrokeType(2.0f));
}
