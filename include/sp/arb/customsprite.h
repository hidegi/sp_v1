#ifndef CUSTOM_SPRITE_H
#define CUSTOM_SPRITE_H
#include <sp/gxsp/drawable.h>
#include <memory>

namespace sp
{
    class CustomSprite : public Drawable
    {
        public:
            typedef std::shared_ptr<CustomSprite>             Ptr;
            typedef std::shared_ptr<const CustomSprite>       ConstPtr;

            static Ptr          create();
            static Ptr          create(const Texture& texture);
            static Ptr          create(const Texture& texture, const recti& area);

            void                setPosition(float x, float y) override;
            void                setPosition(const vec2f& pos);
            void                setTexture(const sp::Texture& texture);
            void                setTextureSprite(const Texture& texture, bool resetRect = false);
            void                setTextureRect(const recti& area);

            void                setSize(const vec2f size);
            void                setSize(float x, float y);
            void                setColor(const Color& color);
            const Texture*      getTexture() const;
            const rectf&        getTextureRect() const;
            const Color&        getColor() const;


        private:
                                CustomSprite();
            explicit            CustomSprite(const Texture& texture);
                                CustomSprite(const Texture& texture, const recti& area);

            void                add(const sp::Vertex& vertex) override;
            void                updatePositions();
            void                updateTexCoords();


    };
}
#endif // SPRITE_H
