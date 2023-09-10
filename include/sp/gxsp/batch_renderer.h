#ifndef BATCH_RENDERER_H
#define BATCH_RENDERER_H
#include <sp/gxsp/color.h>
#include <sp/gxsp/viewport.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/blending.h>
#include <sp/gxsp/render_states.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/framebuffer.h>
#include <sp/math/vec.h>
#include <sp/math/rect.h>
#include <memory>

///TODO: custom drawable states..
namespace sp
{
    class Controller;
    class Meta;
    class Batch;
    enum SP_Invalidation : char
    {
        SP_ALL           = 0x7f,
        SP_VERTEX_BIT    = 1 << 0,
        SP_COLOR_BIT     = 1 << 1,
        SP_TEX_COORD_BIT = 1 << 2,
        SP_INDEX_BIT     = 1 << 3
    };

    /**
     *
     *  - maps..
     *  - lights..
     *      e.g.
     *          light.setParams(position, intensity);
     *          light.setParams(intensity, 50);
     *          light.setColor(r, g, b);
     *
     *  - gui..
     *  - transitions..
     *
     *  can do:
     *  - draw sprites for given z-order..
     */

    //can i please not allocate memory for every drawable??
    class SP_API Renderer
    {
        public:
                           ~Renderer();

            unsigned int    getFramebufferTexHandleGL() const;

            void            setFramebufferPosition(float x, float y);
            void            clear(const Color& color = {0, 0, 0, 0});
            void            setSurfaceSize(unsigned int width, unsigned int height);
            void            setSurfaceSize(const vec2u& dim);

            void            setViewport(const rectf& viewport);
            const Viewport& getViewport() const;
            const Viewport& getDefaultViewport() const;

            void            setAlphaThreshold(float threshold);
            void            setPostProcessShader(const sp::Shader* shader);
            //void            addBatchDrawable();
            void            addDrawable(const Drawable::Ptr primitive, bool set_max);
            void            addDrawable(      Drawable::DrawableStates::Ptr primitive, bool set_max);

            void            removeDrawable(const Drawable::Ptr primitive);
            void            removeDrawable(      Drawable::DrawableStates::Ptr ptr);

            void            resetStatesGL();
            void            invalidate(char = 0x7f);
            void            draw();
            void            draw(Drawable::Ptr drawable);

        private:
                            Renderer();
                            Renderer(const vec2u& screen_size);
                            Renderer(unsigned width, unsigned height);

            friend class sp::Controller;

            void            debugPrint();
            void            removeMetaObject(long int id);
            void            applyCurrentView();
            void            applyBlending(const Blending& b);
            void            applyShader(const Shader* shader);
            void            applyTexture(const Texture* shader);

            void            removeDanglingDrawables();
            void            setupDraw();
            void            cleanupDraw();

            void            swap(Drawable& p1, Drawable& p2);
            void            refresh();

            void            initialize();
            void            ensureResize();

            vec2f               mapPixelsToCoords(int x, int y);
            vec2i               mapCoordsToPixels(float x, float y);

        private:
            void            drawFrame(float x,
                                      float y,
                                      float width,
                                      float height,
                                      const sp::Texture* texture = nullptr,
                                      const sp::Shader* shader = nullptr,
                                      Viewport* viewport = nullptr);

            sp::Viewport    m_default_view;

            //SP_Cache        m_cache;
            struct SP_Cache
            {
                bool            gl_states_set;
                float           alpha_threshold;
                bool            viewport_change;
                Blending        last_blend_mode;
                const Texture*  last_texture;
                const Shader*   last_shader;
            };

            //fixed data..
            std::vector<vec2f>              m_positions;
            std::vector<Color>              m_colors;
            std::vector<vec2f>              m_tex_coords;
            std::vector<vec2f>              m_particles;

            //mutable data..
            std::vector<unsigned int>       m_indices;
            std::vector<Meta>               m_drawables;

            std::vector<Batch>  m_batches;
            size_t                          m_vertex_count;
            size_t                          m_index_count;
            vec2u                           m_size;

            size_t                          m_max_index;
            int                             m_max_zorder;

            size_t                          m_particle_count;

            int                             m_index_refresh_count;
            bool                            m_refresh_vertices;
            bool                            m_refresh_colors;
            bool                            m_refresh_tex_coords;

            bool                            m_index_resize;
            Blending                        m_blend_mode;
            SP_Cache                        m_cache;

            vec2f                           m_frame_position;
            const sp::Shader*               m_post_process_shader;

            mutable Framebuffer             m_primary_framebuffer;
            mutable Framebuffer             m_secondary_framebuffer;

            mat                             m_view_matrix;
            mat                             m_inv_view_matrix;
    };
}
#endif // BATCH_RENDERER_H
