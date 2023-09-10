#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

namespace sp
{
    class AbstractBatchRenderer
    {
        public:
            unsigned int        getFramebufferTexHandleGL();
            void                setFramebufferPosition(float x, float y);
            void                clear(const Color& color = {0, 0, 0, 0});
            void                setSurfaceSize(unsigned int width, unsigned int height);
            void                setViewport(const rectf& viewport);
            const Viewport&     getViewport() const;
            const Viewport&     getDefaultViewport();


            void                resetStatesGL();
            virtual void        draw() = 0;

        protected:
            virtual void        refresh() = 0;

            mutable Framebuffer     m_framebuffer;
            const sp::Shader*       m_post_process_shader;

        protected:
            struct SP_Cache
            {
                bool                gl_states_set;
                float               alpha_threshold;
                bool                viewport_change;
                Blending            last_blend_mode;
                const Texture*      last_texture;
                const Shader*       last_shader;
            };

            std::vector<vec2f>          m_positions;
            std::vector<Color>          m_colors;
            std::vector<vec2f>          m_tex_coords;
            std::vector<unsigned int>   m_indices;

            int                         m_index_refresh_count;
            vec2u                       m_virtual_size;
    };
}
#endif // ABSTRACTRENDERER_H
