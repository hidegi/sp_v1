#ifndef VIEWPORT_H
#define VIEWPORT_H
#include <sp/math/rect.h>
#include <sp/math/vec.h>

namespace sp
{
    class SP_API Viewport
    {
        public:
                            Viewport();
                            Viewport(const rectf& vp);
                            Viewport(const vec2f& pos, const vec2f& dim);
                            Viewport(const Viewport& vp);
                            Viewport(Viewport&& vp);
                           ~Viewport() = default;

            Viewport&       operator=(const Viewport& vp);
            Viewport&       operator=(Viewport&& vp) noexcept;

            friend bool     operator==(const Viewport& vp1, const Viewport& vp2);
            friend bool     operator!=(const Viewport& vp1, const Viewport& vp2);
            void            setOrigin(float x, float y);
            void            setOrigin(const vec2f& pos);

            void            setSize(float w, float h);
            void            setSize(const vec2f& dim);

            void            setViewport(const rectf& rect);


            const vec2f&    getOrigin() const;
            const vec2f&    getSize() const;

            void            translate(float x, float y);
            void            translate(const vec2f& offset);

            void            focus(float factor);

            void            load() const;
            bool            defaulted() const;

            const float*    getMatrix() const;
        private:
            void            resetViewport();
            bool            m_default;
            recti           m_viewport;
            vec2f           m_origin;
            vec2f           m_size;
            float           m_matrix[16];
    };
}
#endif // VIEWPORT_H
