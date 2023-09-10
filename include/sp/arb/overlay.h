#ifndef OVERLAY_H
#define OVERLAY_H
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/sprite.h>
#include <memory>

namespace sp
{
    //helper class for owning a drawable..
    //the idea is that this class will maintain one vector for a series
    //of drawables, reducing costs of performing numerous allocations
    //for a crowded amount of drawables..

    //of course, each element must have the properties of an actual drawable..
    //this class must effectively emulate this feature..

    //intended for multiple overlay drawable objects..
    class SP_API SpriteBatch : public Drawable
    {
        public:
            typedef std::shared_ptr<SpriteBatch>        Ptr;
            typedef std::shared_ptr<const SpriteBatch>  ConstPtr;

            static Ptr                  create();
            int                         addQuad(const Texture& texture,
                                                const vec2f& pos,
                                                const vec2f& size = {},
                                                const recti& area = recti{});

            void                        uniformTranslate(const vec2f& pos);
            void                        uniformTranslate(float x, float y);
            void                        uniformVisibility(bool visible);

            void                        setPosition(int index, const sp::vec2f& pos);
            void                        setColor(int index, const sp::Color& color);

            void                        removeSprite(int index);
            bool                        isVisible(int index) const;
            void                        setVisible(int index, bool visible);

            const Sprite&               get(int index) const;
            Sprite&                     get(int index);

            size_t                      size() const;

        private:
                                        SpriteBatch();
            virtual void addDrawable(Renderer& renderer, bool max = false) override;
            virtual void removeDrawable(Renderer& renderer) override;
            std::vector<Sprite>         m_sprites;

            //std::vector<DrawStates>     m_drawStates;
            mutable DrawableStates      m_draw_states;
            vec2f                       m_origin;
    };
}

#endif // OVERLAY_H
