#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AngularLookAndFeel : public LookAndFeel_V3 {
public:
    static Colour create_base_colour(Colour button_colour,
                                     bool has_keyboard_focus,
                                     bool is_mouse_over,
                                     bool is_button_down) noexcept;
    static void matte_background_box(Graphics& g,
                                     Rectangle<int> bounds,
                                     bool vertical,
                                     const Colour& colour);
    static void matte_foreground_box(Graphics& g,
                                     Rectangle<int> bounds,
                                     const Colour& colour);
    static void matte_box(Graphics& g,
                          const Rectangle<int>& bounds,
                          bool vertical,
                          const Colour& colour);
    AngularLookAndFeel();

    void drawProgressBar(Graphics& g,
                         juce::ProgressBar& bar,
                         int width,
                         int height,
                         double progress,
                         const String& textToShow) override;

    void drawButtonBackground(Graphics& g,
                              Button& button,
                              const Colour& background,
                              bool is_mouse_over,
                              bool is_button_down) override;

    void drawStretchableLayoutResizerBar(Graphics& g,
                                         int w,
                                         int h,
                                         bool is_vertical,
                                         bool is_mouse_over,
                                         bool is_mouse_dragging) override;

    void fillTextEditorBackground(Graphics& g,
                                  int width,
                                  int height,
                                  TextEditor&) override;

    void drawTextEditorOutline(Graphics& g,
                               int width,
                               int height,
                               TextEditor&) override;

    void drawGroupComponentOutline(Graphics& g,
                                   int width,
                                   int height,
                                   const String& text,
                                   const Justification& position,
                                   GroupComponent& group) override;

    void drawPropertyPanelSectionHeader(Graphics& g,
                                        const String& name,
                                        bool is_open,
                                        int width,
                                        int height) override;

    void drawPropertyComponentBackground(Graphics& g,
                                         int width,
                                         int height,
                                         PropertyComponent& p) override;

    void drawLinearSliderBackground(Graphics& g,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    float slider_pos,
                                    float min_slider_pos,
                                    float max_slider_pos,
                                    const Slider::SliderStyle,
                                    Slider&) override;

    void drawLinearSliderThumb(Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float slider_pos,
                               float min_slider_pos,
                               float max_slider_pos,
                               const Slider::SliderStyle,
                               Slider& s) override;

    void drawComboBox(Graphics& g,
                      int w,
                      int h,
                      bool button_down,
                      int button_x,
                      int button_y,
                      int button_w,
                      int button_h,
                      ComboBox& cb) override;

    void drawCallOutBoxBackground(CallOutBox& box,
                                  Graphics& g,
                                  const Path& path,
                                  Image& cachedImage) override;

    void drawTickBox(Graphics& g,
                     Component& component,
                     float x,
                     float y,
                     float w,
                     float h,
                     const bool ticked,
                     const bool isEnabled,
                     const bool isMouseOverButton,
                     const bool isButtonDown) override;

    static const Colour emphasis;
};