#ifndef LEVLER_H
#define LEVLER_H
#include <sp/gxsp/texture.h>
#include <sp/gxsp/transformable.h>
#include <sp/arb/overlay.h>
#include <sp/math/vec.h>
#include <sp/string.h>
#include <set>
#include <map>

namespace sp
{
    //creates an output texture for a level..
    //provide it a rule set file..

    struct Tile
    {
        int     index;
        int     id;
        bool    walkable;
    };
    class SP_API Levler
    {
        private:
            using Predicate = std::function<bool(float, float)>;

            struct Evaluator
            {
                int     id;
                vec2u   texCoords;
                bool    walkable;
                bool    empty;
                bool    overlay;
            };


        public:
            //just add const qualifiers later pls..
            explicit            Levler();
                               ~Levler();
            void                loadRuleSet(const char* path);
            void                loadLevel(const void* data);
            void                setSourceTexture(const Texture& texture);
            void                setDrawableUnitLength(unsigned int length);


            void                setViewport(const sp::rectf& vp);
            void                setVisible(int index, bool visible);
            bool                isVisible(int index) const;

            int                 getIndex(float x, float y);
            int                 getIndex(const vec2f& pos);


            bool                walkable(const vec2f& pos);
            bool                walkable(float x, float y);

            bool                isOutOfBounds(const sp::vec2f& pos);
            bool                isOutOfBounds(float x, float y);

            const Tile&         getTile(const sp::vec2f& pos);
            const Tile&         getTile(float x, float y);

            vec2f               mapToViewport(float x, float y);
            vec2f               mapToViewport(const vec2f& pos);

            vec2u               getMapSize() const;
            sp::Texture         getOutputTexture();
            sp::Sprite::Ptr     getOutputSprite();

            unsigned int        getDrawableUnitLength();
            unsigned int        getOverlayDrawableCount() const;

            vec2i               mapToGrid(float x, float y);
            vec2i               mapToGrid(const vec2f& pos);
            vec2f               getOrigin() const;
            void                setKeyPosition(const sp::vec2i& pos, Predicate predicate);


            sp::SpriteBatch::Ptr     getSpriteBatch();
        private:


            vec2f               mapPixels(float x, float y);
            vec2f               mapPixels(const vec2f& pos);

            vec2i               mapCoords(float x, float y);
            vec2i               mapCoords(const vec2f& pos);


            void                parseSection(const sp::String& ex);

            vec2u               m_dimensions;
            const Texture*      m_texture_atlas;
            Texture             m_output_texture;
            Transformable       m_transformable;

            rectf               m_bounds;
            unsigned int        m_padding;
            unsigned int        m_sprite_unit_length;
            unsigned int        m_tex_unit_length;
            unsigned int        m_overlay_count;

            SpriteBatch::Ptr    m_sprite_batch;
            Sprite::Ptr         m_output_sprite;

            std::map<int, Predicate> m_key_positions;
            std::map<int, Evaluator> m_rule_set;
            std::vector<Tile>   m_grid;
    };
}

#endif // LEVLER_H
