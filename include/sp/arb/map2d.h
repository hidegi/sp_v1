#ifndef MAP2D_H
#define MAP2D_H
#include <sp/gxsp/color.h>
#include <sp/gxsp/texture.h>
#include <sp/gxsp/sprite.h>
#include <sp/math/vec.h>

namespace sp
{
    class Map2D
    {
        public:
            struct Tile
            {
                int         type;
                vec2u       tex_coords;
                sp::Color   color;
                bool        walkable;
            };
        public:
            //parser..
                                    Map2D();
            void                    loadRuleSet(const char* path);
            void                    loadLevel(const void* data);
            void                    setTextureAtlas(const Texture& texture);
            void                    markAsDrawable(unsigned int token);


            bool                    walkable(const vec2f& position);
            bool                    walkable(float x, float y);

            const sp::Texture&      getLevelTexture() const;
            sp::Sprite::Ptr         getSprite() const;
        private:

            vec2u                   mapPosition(const vec2f& position);

            void                    drawTile(const Tile& tile);
            Tile                    parseDelimiter(sp::String str, int& _id);

        private:
            vec2f                   mapPixels(const vec2f& external);
            rectf                   m_bounds;
            vec2u                   m_dimensions;

            Viewport                m_viewport;

            const Texture*          m_texture_atlas;
            Texture                 m_grid_texture;

            sp::Sprite::Ptr         m_output_sprite;
            std::vector<bool>       m_grid;
            unsigned int            m_padding;
            unsigned int            m_unit_length;

            mat                     m_matrix;
            std::map<int, Tile>     m_rule_set;
    };
}
#endif // MAP2D_H
