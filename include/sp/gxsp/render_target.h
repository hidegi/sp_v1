#ifndef RENDERER_H
#define RENDERER_H
#include <sp/sp.h>
#include <sp/gxsp/color.h>
#include <sp/gxsp/viewport.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/vertex.h>
#include <sp/gxsp/shader.h>
#include <sp/math/mat.h>

namespace sp
{
    class SP_API Target
    {
        public:
                        Target();
            virtual    ~Target();

            void        clear(const Color& color = {0, 0, 0, 0});

            void        setSurfaceSize(unsigned int width, unsigned int height);
            void        setSurfaceSize(vec2u dim);

            void        setViewport(const rectf& viewport);
            const Viewport& getViewport() const;
            const Viewport& getDefaultViewport() const;

            void        draw(const Drawable& draw, const States& states = States::DEFAULT);
            void        draw(const Vertex* vertices, size_t length, int type, const States& states = States::DEFAULT);
            vec2u       getSize() const;
            void        pushStatesGL();
            void        popStatesGL();
            void        resetStatesGL();

            //void      mapCoordsToPixel(vec2f point, const Viewport& view) const;
            void        initialize();

        private:
            void        applyCurrentView();
            void        applyTransform(const mat& m);
            void        applyBlending(const Blending& b);
            void        applyTexture(const Texture* texture);
            void        applyShader(const Shader* shader);
            void        setupDraw(bool cache, const States& states);
            void        drawMesh(int type, size_t first, size_t length);
            void        cleanupDraw(const States& states);

            struct SP_Cache
            {
                enum
                {CACHE_SIZE = 4};

                bool        enable;
                bool        gl_states_set;
                bool        viewport_change;
                Blending    last_blend_mode;
                SPuint64    last_texture_id;
                bool        use_client_texcoords;
                bool        use_vertex_cache;
                Vertex      vertex_cache[CACHE_SIZE];
            };
            Viewport    m_default_view;
            Viewport    m_custom_view;
            SP_Cache    m_cache;
            SPuint64    m_id;   //ok..

            vec2u       m_size;
    };
}
#endif // RENDERER_H
