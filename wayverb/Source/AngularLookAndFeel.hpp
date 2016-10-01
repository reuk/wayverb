#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

class AngularLookAndFeel : public juce::LookAndFeel_V3 {
public:
    static juce::Colour create_base_colour(juce::Colour button_colour,
                                     bool has_keyboard_focus,
                                     bool is_mouse_over,
                                     bool is_button_down) noexcept;
    static void matte_background_box(juce::Graphics& g,
                                     juce::Rectangle<int> bounds,
                                     bool vertical,
                                     const juce::Colour& colour);
    static void matte_foreground_box(juce::Graphics& g,
                                     juce::Rectangle<int> bounds,
                                     const juce::Colour& colour);
    static void matte_box(juce::Graphics& g,
                          const juce::Rectangle<int>& bounds,
                          bool vertical,
                          const juce::Colour& colour);
    AngularLookAndFeel();

    void drawProgressBar(juce::Graphics& g,
                         juce::ProgressBar& bar,
                         int width,
                         int height,
                         double progress,
                         const juce::String& textToShow) override;

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& background,
                              bool is_mouse_over,
                              bool is_button_down) override;

    void drawStretchableLayoutResizerBar(juce::Graphics& g,
                                         int w,
                                         int h,
                                         bool is_vertical,
                                         bool is_mouse_over,
                                         bool is_mouse_dragging) override;

    void fillTextEditorBackground(juce::Graphics& g,
                                  int width,
                                  int height,
                                  juce::TextEditor&) override;

    void drawTextEditorOutline(juce::Graphics& g,
                               int width,
                               int height,
                               juce::TextEditor&) override;

    void drawGroupComponentOutline(juce::Graphics& g,
                                   int width,
                                   int height,
                                   const juce::String& text,
                                   const juce::Justification& position,
                                   juce::GroupComponent& group) override;

    void drawPropertyPanelSectionHeader(juce::Graphics& g,
                                        const juce::String& name,
                                        bool is_open,
                                        int width,
                                        int height) override;

    void drawPropertyComponentBackground(juce::Graphics& g,
                                         int width,
                                         int height,
                                         juce::PropertyComponent& p) override;

    void drawLinearSliderBackground(juce::Graphics& g,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    float slider_pos,
                                    float min_slider_pos,
                                    float max_slider_pos,
                                    const juce::Slider::SliderStyle,
                                    juce::Slider&) override;

    void drawLinearSliderThumb(juce::Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float slider_pos,
                               float min_slider_pos,
                               float max_slider_pos,
                               const juce::Slider::SliderStyle,
                               juce::Slider& s) override;

    void drawComboBox(juce::Graphics& g,
                      int w,
                      int h,
                      bool button_down,
                      int button_x,
                      int button_y,
                      int button_w,
                      int button_h,
                      juce::ComboBox& cb) override;

    void drawCallOutBoxBackground(juce::CallOutBox& box,
                                  juce::Graphics& g,
                                  const juce::Path& path,
                                  juce::Image& cachedImage) override;

    void drawTickBox(juce::Graphics& g,
                     juce::Component& component,
                     float x,
                     float y,
                     float w,
                     float h,
                     const bool ticked,
                     const bool isEnabled,
                     const bool isMouseOverButton,
                     const bool isButtonDown) override;

    static const juce::Colour emphasis;
};