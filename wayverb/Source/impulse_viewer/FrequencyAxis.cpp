#include "FrequencyAxis.hpp"

namespace {

class TexturedQuad : public ::Drawable {
public:
    TexturedQuad(TexturedQuadShader& shader)
            : shader(&shader)
            , geometry(std::vector<glm::vec3>{{-0.5, -0.5, 0},
                                              {0.5, -0.5, 0},
                                              {-0.5, 0.5, 0},
                                              {0.5, 0.5, 0}})
            , uv(std::vector<glm::vec2>{{0, 0}, {1, 0}, {0, 1}, {1, 1}})
            , ibo(std::vector<GLushort>{0, 1, 2, 3}) {
        auto s_vao = vao.get_scoped();

        auto v_position = shader.get_attrib_location("v_position");
        glEnableVertexAttribArray(v_position);
        geometry.bind();
        glVertexAttribPointer(v_position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        auto v_uv = shader.get_attrib_location("v_uv");
        glEnableVertexAttribArray(v_uv);
        uv.bind();
        glVertexAttribPointer(v_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        ibo.bind();
    }

    void draw() const override {
        auto s_shader = shader->get_scoped();
        auto s_vao = vao.get_scoped();
        glDrawElements(
                GL_TRIANGLE_STRIP, ibo.size(), GL_UNSIGNED_SHORT, nullptr);
    }

private:
    TexturedQuadShader* shader;

    VAO vao;
    StaticVBO geometry;
    StaticVBO uv;
    StaticIBO ibo;
};

}  // namespace

//----------------------------------------------------------------------------//

TexturedQuadShader::TexturedQuadShader()
        : ShaderProgram(vert, frag) {
}

void TexturedQuadShader::set_model_matrix(const glm::mat4& m) const {
    set("v_model", m);
}
void TexturedQuadShader::set_view_matrix(const glm::mat4& m) const {
    set("v_view", m);
}
void TexturedQuadShader::set_projection_matrix(const glm::mat4& m) const {
    set("v_projection", m);
}

void TexturedQuadShader::set_tex(GLint i) const {
    set("f_tex", i);
}

const std::string TexturedQuadShader::vert{R"(
#version 150
in vec3 v_position;
in vec2 v_uv;
out vec2 f_uv;
uniform vec3 v_billboard;
uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;
void main() {
    gl_Position = v_projection * v_view * v_model * vec4(v_position, 1.0);
    f_uv = v_uv;
}
)"};

const std::string TexturedQuadShader::frag{R"(
#version 150
in vec2 f_uv;
out vec4 frag_color;
uniform sampler2D f_tex;
void main() {
    frag_color = texture(f_tex, f_uv);
}
)"};

//----------------------------------------------------------------------------//

void TextImage::set_text(const std::string& text, int height) {
    Font font(height);
    auto width = font.getStringWidth(text);

    auto p_width = std::pow(2, std::ceil(std::log2(width)));
    auto p_height = std::pow(2, std::ceil(std::log2(height)));

    auto p_dim = std::max(p_width, p_height);

    image = Image(Image::ARGB, p_dim, p_dim, true);
    {
        Graphics g(image);
        g.setFont(font);
        g.setColour(Colours::white);
        g.drawText(text, 0, 0, p_dim, p_dim, Justification::centred);
    }
}

const Image& TextImage::get_image() const {
    return image;
}

//----------------------------------------------------------------------------//

FrequencyAxisObject::FrequencyAxisObject(ShaderProgram& shader,
                                         TexturedQuadShader& quad_shader)
        : BasicDrawableObject(
                  shader,
                  {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}, {0, 1, 0}},
                  {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},
                  {0, 1, 2, 3},
                  GL_LINES)
        , quad_shader(&quad_shader) {
    set_label("text or whatever");
}

void FrequencyAxisObject::set_label(const std::string& t) {
    TextImage text_image;
    text_image.set_text(t, 32);
    texture.loadImage(text_image.get_image());
}

void FrequencyAxisObject::draw() const {
    BasicDrawableObject::draw();

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    texture.bind();

    TexturedQuad quad(*quad_shader);

    auto s = quad_shader->get_scoped();
    quad_shader->set_tex(0);

    quad.draw();

    glDisable(GL_TEXTURE_2D);
}