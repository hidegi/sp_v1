#ifndef RADIALLIGHT_H
#define RADIALLIGHT_H
#include <sp/sp.h>
#include <sp/lxsp/light_source.h>
#include <sp/gxsp/framebuffer.h>
#include <sp/gxsp/transformable.h>

namespace sp
{
    class RadialLight : public LightSource
    {
        public:
            typedef std::shared_ptr<RadialLight>        Ptr;
            typedef std::shared_ptr<const RadialLight>  ConstPtr;

            static Ptr      create();
                           ~RadialLight();
            void            cast(const EdgeVector::iterator& begin, const EdgeVector::iterator& end) override;
            void            setBeamAngle(float angle);
            float           getBeamAngle() const;

            virtual void    setPosition(float x, float y) override;
        private:
                            RadialLight();

            virtual rectf   getGlobalBounds() const override;
            virtual void    initializeTextures() override;
            void            resetColor();
            void                    draw();
            float                   m_beam_angle;
            static sp::Texture      fade_texture;

            static int              instance_count;
            mutable Transformable   m_transformable;
            CustomDrawStates        m_draw_states;

    };
}
#endif // RADIALLIGHT_H
