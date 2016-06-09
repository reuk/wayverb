#include "TextHandler.hpp"

#include "modern_gl_utils/sampler.h"

#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)

#include "ft2build.h"
#include FT_FREETYPE_H

class TextHandler::Impl {
public:
    Impl(TextShader& shader)
            : shader(&shader)
            , uv(std::vector<GLfloat>{0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1})
            , ibo(std::vector<GLushort>{0, 1, 2, 3, 4, 5}) {
        //  set up freetype
        auto error = FT_Init_FreeType(&library);
        assert(!error);
        error = FT_New_Face(library, "Hack-Regular.otf", 0, &face);
        assert(!error);
    }

    void configure_vao(const StaticVBO& geometry) const {
        auto s_vao = vao.get_scoped();

        auto v_position = shader->get_attrib_location("v_position");
        glEnableVertexAttribArray(v_position);
        geometry.bind();
        glVertexAttribPointer(v_position, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        auto v_uv = shader->get_attrib_location("v_uv");
        glEnableVertexAttribArray(v_uv);
        uv.bind();
        glVertexAttribPointer(v_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        ibo.bind();
    }

    void draw(const std::string& s,
              int pixel_height,
              const glm::vec2& position) const {
        FT_Set_Pixel_Sizes(face, 0, pixel_height);

        auto pos = position;
        glm::vec2 scale_factor{2.0 / 500};

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (const auto& i : s) {
            auto error = FT_Load_Char(face, i, FT_LOAD_RENDER);
            assert(!error);

            Texture tex;
            glActiveTexture(GL_TEXTURE0);
            tex.data(face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     face->glyph->bitmap.buffer);

            Sampler samp;
            samp.set_where(0);
            auto s_sampler = samp.get_scoped();
            samp.parameter_i(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            samp.parameter_i(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            samp.parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            samp.parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            auto s_shader = shader->get_scoped();
            shader->set_tex(0);

            auto v = pos +
                     glm::vec2(face->glyph->bitmap_left,
                               face->glyph->bitmap_top) *
                             scale_factor;
            auto u = glm::vec2(face->glyph->bitmap.width,
                               face->glyph->bitmap.rows) *
                     scale_factor;

            StaticVBO geometry(std::vector<GLfloat>{v.x,
                                                    v.y,
                                                    v.x,
                                                    v.y - u.y,
                                                    v.x + u.x,
                                                    v.y,
                                                    v.x + u.x,
                                                    v.y,
                                                    v.x,
                                                    v.y - u.y,
                                                    v.x + u.x,
                                                    v.y - u.y});
            configure_vao(geometry);

            auto s_vao = vao.get_scoped();
            glDrawElements(
                    GL_TRIANGLES, ibo.size(), GL_UNSIGNED_SHORT, nullptr);

            pos += glm::vec2(face->glyph->advance.x, face->glyph->advance.y) *
                   scale_factor / 64.0f;
        }
    }

    virtual ~Impl() noexcept {
        auto error = FT_Done_Face(face);
        error = FT_Done_FreeType(library);
    }

private:
    TextShader* shader;

    VAO vao;
    StaticVBO uv;
    StaticIBO ibo;

    FT_Library library;
    FT_Face face;
};

//----------------------------------------------------------------------------//

TextHandler::TextHandler(TextShader& shader)
        : pimpl(std::make_unique<Impl>(shader)) {
}

TextHandler::~TextHandler() noexcept = default;

void TextHandler::draw(const std::string& s,
                       int pixel_height,
                       const glm::vec2& position) const {
    pimpl->draw(s, pixel_height, position);
}