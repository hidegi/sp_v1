#ifndef FONT_H
#define FONT_H
#include <sp/sp.h>
#include <sp/math/vec.h>
#include <sp/math/rect.h>
#include <sp/gxsp/texture.h>
#include <memory>
#include <map>

namespace sp
{
    struct SP_API SP_Char
    {
        unsigned short x0, y0, x1, y1;
        float xoff, yoff, advance;
    };

    struct SP_API SP_Character
    {
        unsigned int    codepoint;
        float           advance;
        vec2f           bearing;
        vec2i           size;
        rectf           bounds;
        rectf           tex_coords;
    };

    struct SP_API SP_Font_Map
    {
        float       ascent;
        float       descent;
        float       linegap;
        float       scale;
        float       linespacing;
        vec2i       size;
        sp::Texture texture;
        private:
            friend class    Font;
            SP_Char         char_data[255];
    };

    class SP_API Font : public std::enable_shared_from_this<Font>
    {
        public:
            typedef std::shared_ptr<Font>           Ptr;
            typedef std::shared_ptr<const Font>     ConstPtr;

           ~Font();
            static Ptr      create();
                   Ptr      get();

            bool                loadFromFile(const char* filename);
            bool                loadFromMemory(const void* data);
            const SP_Character& getCharInfo(unsigned int codepoint, unsigned int char_size);
            int                 getKerning(unsigned int first, unsigned int second);
            const sp::Texture&  getTexture(unsigned int charSize) const;
            const SP_Font_Map&  getMap(unsigned int char_size);

        private:

            Font();
            SP_Font_Map&        createFontMap(unsigned int char_size);
            SP_Character        createCharacter(unsigned int codepoint, unsigned char_size);

        private:
            void*   m_font_info;
            void*   m_file_data;
            bool    m_owning;

                    std::map<SPuint64, SP_Character>    m_char_map;
            mutable std::map<unsigned int, SP_Font_Map> m_maps;
    };
}

#endif // FONT_H
