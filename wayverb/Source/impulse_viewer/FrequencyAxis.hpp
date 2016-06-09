#pragma once

#include "BasicDrawableObject.hpp"
#include "TextHandler.hpp"

class FrequencyAxisObject final : public BasicDrawableObject {
public:
    FrequencyAxisObject(ShaderProgram& shader, TextShader& text_shader);

    void set_label(const std::string& t);

    void draw() const override;

private:
    TextHandler text_handler;
    std::string label{"text goes here"};
};
