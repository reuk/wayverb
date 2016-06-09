#pragma once

#include "modern_gl_utils/buffer_object.h"
#include "modern_gl_utils/text_shader.h"
#include "modern_gl_utils/vao.h"

#include "glm/glm.hpp"

class TextHandler {
public:
    TextHandler(TextShader& shader);
    virtual ~TextHandler() noexcept;

    void draw(const std::string& s,
              int pixel_height,
              const glm::vec2& position) const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
