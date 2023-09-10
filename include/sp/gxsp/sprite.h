#ifndef SPRITE_H
#define SPRITE_H
#include <sp/gxsp/drawable.h>

namespace sp
{
    class SP_API Sprite : public Drawable
    {
        public:
            typedef std::shared_ptr<Sprite>             Ptr;
            typedef std::shared_ptr<const Sprite>       ConstPtr;

                               ~Sprite();
            static Ptr          create();
            static Ptr          create(const Texture& texture);
            static Ptr          create(const Texture& texture, const recti& area);

            void                setOrigin(float x, float y);
            void                setOrigin(const vec2f& orig);
            const vec2f&        getOrigin() const;

            void                setPosition(float x, float y) override;
            void                setPosition(const vec2f& pos);
            void                setTexture(const sp::Texture& texture);
            void                setTextureSprite(const Texture& texture, bool resetRect = false);
            void                setTextureRect(const recti& area);

            void                setScale(const vec2f& scale);
            void                setScale(float x, float y);
            void                setViewport(const rectf& viewport);
            const Viewport&     getViewport() const;


            void                setSize(const vec2f size);
            void                setSize(float x, float y);
            void                setColor(const Color& color);
            const Texture*      getTexture() const;
            const rectf&        getTextureRect() const;
            const Color&        getColor() const;


        private:
            friend class SpriteBatch;

                                Sprite();
            explicit            Sprite(const Texture& texture);
                                Sprite(const Texture& texture, const recti& area);

            virtual void        addDrawable(Renderer& renderer, bool max) override;
            void                add(const sp::Vertex& vertex) override;
            void                updatePositions();
            void                updateTexCoords();

            rectf               m_tex_coords_bounds;
            vec2f               m_origin;
            Viewport            m_viewport;
            bool                m_custom_size;
    };
}
#endif // SPRITE_H
