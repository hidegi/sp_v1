#ifndef LIGHTAREA_H
#define LIGHTAREA_H
#include <sp/lxsp/line.h>
#include <sp/lxsp/light_source.h>
#include <sp/gxsp/drawable.h>

namespace sp
{
    //rectangle..
    class LightArea : public sp::Drawable
    {
        public:
            enum Mode
            {
                FOG,
                AMBIENT
            };

            typedef std::shared_ptr<LightArea>        Ptr;
            typedef std::shared_ptr<const LightArea>  ConstPtr;

            static Ptr          create(Mode mode, const vec2f& pos, const vec2f& dim);
            static Ptr          create(Mode mode, const sp::Texture* texture, sp::recti = recti{});



            void        setAreaColor(const Color& color);
            Color       getAreaColor() const;

            void        setAreaOpacity(float opacity);
            float       getAreaOpacity() const;

            void        setAreaTexture() const;
            void        setTextureRect(const sp::recti& ret);
            recti       getTextureRect() const;

            void        setMode(Mode mode);
            Mode        getMode() const;

            //void        clear();
        private:
                        LightArea(Mode mode, const vec2f& pos, const vec2f& dim);
                        LightArea(Mode mode, const sp::Texture* texture, sp::recti);

            void        draw();

            Texture     m_texture;
            recti       m_base_texture_rect;
            Color       m_color;
            float       m_opacity;
            vec2f       m_size;
            Mode        m_mode;
    };
}

#endif // LIGHTAREA_H
