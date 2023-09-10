#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <sp/sp.h>
#include <sp/math/vec.h>
#include <sp/gxsp/texture.h>
#include <sp/gxsp/color.h>
//#include <sp/gxsp/render_target.h>
#include <sp/gxsp/sprite.h>

namespace sp
{
    class SP_API Framebuffer
    {
        public:
                                Framebuffer();
                               ~Framebuffer();

            bool                create(unsigned int width, unsigned int height, bool multisample = true);
            void                bind();
            void                display();

            void                createColorAttachment();
            //void            setClearColor(const sp::Color& color);
            //void                clear(const sp::Color& color = {0, 0, 0, 0});
            //void                draw(const Drawable& drawable, const States& states = States::DEFAULT) const;
            const sp::Sprite&   getSprite() const;

            unsigned int        getTexHandleGL();
            sp::Texture         copyColorToTexture();
            void                destroy();
            void                clear(const sp::Color& color = sp::Color{0, 0, 0, 0});

            const sp::Texture&  getColorTexture();
            //void            update();

            void                clearArea(const recti& area);
            void                clearArea(int t, int l, int w, int h);
            void                clearArea(const vec2i& tl, const vec2i& wh);
            const SPuint8*      getColorBuffer();
            void                draw(
                                    const Vertex* vertices,
                                    size_t length,
                                    int primitive_type,
                                    const vec2f& offset = {0.f, 0.f},
                                    const vec2f& scale  = {1.f, 1.f},
                                    float point_size    = 0.f,
                                    Blending blending   = BlendAlpha,
                                    const sp::Texture* texture = nullptr,
                                    const sp::Shader* shader   = nullptr);
        private:
            void                copyColor(unsigned int dstID, int x = 0, int y = 0, int w = 0, int h = 0);

            void                deleteBuffers();
            bool                isComplete();

            unsigned int    m_fbo_obj;
            unsigned int    m_fbo_msaa_obj;

            unsigned int    m_rbo_obj;
            unsigned int    m_rbo_msaa_color_obj;
            unsigned int    m_rbo_msaa_depth_obj;
            unsigned char*  m_color_buffer;

                     int    m_samples;
            unsigned int    m_texture;
            unsigned int    m_current_color_attachment;

            Texture         m_color_texture;
            //Texture         m_dummy_texture;
            //sp:::Color      m_clear_color;
            vec2u           m_size;
    };
}

#endif // FRAMEBUFFER_H
