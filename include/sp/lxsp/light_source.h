#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/color.h>
#include <sp/gxsp/vertex_array.h>
#include <sp/lxsp/line.h>

namespace sp
{
    typedef Line Edge;
    typedef std::vector<Edge> EdgeVector;


    class LightSource : public Drawable
    {
        public:
                                LightSource();
            virtual            ~LightSource();

            void                setIntensity(float intensity);
            float               getIntensity() const;

            void                setColor(const sp::Color& color);
            sp::Color           getColor() const;

            virtual void        setFade(bool fade);
            bool                hasFade() const;

            void                setRange(float range);
            float               getRange() const;

            virtual void        cast(const EdgeVector::iterator& begin, const EdgeVector::iterator& end) = 0;
        protected:
            virtual void        resetColor() = 0;
            virtual void        initializeTextures() = 0;

            VertexArray m_polygon;
            sp::Color   m_color;
            float       m_range;
            float       m_intensity;
            bool        m_fade;
    };
}

#endif // LIGHTSOURCE_H
