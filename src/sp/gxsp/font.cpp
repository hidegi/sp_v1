#include <sp/gxsp/font.h>
#include <sp/exception.h>
#include <sp/gxsp/texture.h>
#include <sp/utils/helpers.h>
#include <sp/math/math.h>
#include <sp/spgl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <cmath>

namespace sp
{
    namespace
    {
        SPuint64 createCipher(unsigned int codepoint, unsigned int char_size)
        {
            SPuint64 charSize   = static_cast<SPuint64>(char_size) << 31;
            SPuint64 code       = static_cast<SPuint64>(codepoint);
            return (charSize | code);
        }
    }

    Font::Font() :
        m_font_info {nullptr},
        m_file_data {nullptr},
        m_owning    {true}
    {
    }

    Font::~Font()
    {
        if(m_file_data && m_owning)
        {
            printf("freeing file data..\n");
            free(m_file_data);
            printf("free ok..\n");
        }
        if(m_font_info) free(m_font_info);
    }

    Font::Ptr Font::create()
    {
        return Ptr(new Font);
    }

    Font::Ptr Font::get()
    {
        try
        {
            return shared_from_this();
        }
        catch(std::bad_weak_ptr const&)
        {
            throw SP_EXCEPTION(L"font is not owned by any smart pointer");
        }
    }

    bool Font::loadFromFile(const char* filename)
    {
        size_t filesize;
        SPuint8* file = spHelperFileData(filename, &filesize);
        if(!file || !loadFromMemory(file))
            return false;
        m_owning = true;
        return true;
    }

    bool Font::loadFromMemory(const void* data)
    {
        m_owning = false;
        m_file_data = const_cast<void*>(data);
        if((stbtt_GetFontOffsetForIndex(reinterpret_cast<const sp_uint8*>(data), 0)) == (unsigned)-1) return false;
        stbtt_fontinfo* font = (stbtt_fontinfo*) calloc(1, sizeof(stbtt_fontinfo));
        stbtt_InitFont(font, static_cast<SPuint8*>(m_file_data), 0);
        m_font_info = font;
        return true;
    }

    const SP_Font_Map& Font::getMap(unsigned int charSize)
    {
        auto it = m_maps.find(charSize);
        if(it == m_maps.end())
            return createFontMap(charSize);
        return it->second;
    }

    int nextP2(unsigned int charSize)
    {
        int nsize = 1;
        while(nsize < (charSize * 10))
            nsize = nsize << 1;

        return std::min(nsize, 1024);
    }

    SP_Font_Map& Font::createFontMap(unsigned int charSize)
    {
        charSize = clamp(charSize, 0x5u, 0x64u);
        int nsize = nextP2(charSize);

        SP_Font_Map& map = m_maps.insert({charSize, {}}).first->second;
        SPuint8 tmpBitmap[nsize * nsize];
        memset(tmpBitmap, 0, nsize * nsize);
        stbtt_BakeFontBitmap(
            static_cast<SPuint8*>(m_file_data),
            0,
            static_cast<float>(charSize),
            tmpBitmap,
            nsize,
            nsize,
            32,
            255,
            reinterpret_cast<stbtt_bakedchar*>(map.char_data));

        map.scale = stbtt_ScaleForPixelHeight(static_cast<stbtt_fontinfo*>(m_font_info), static_cast<float>(charSize));
        map.texture.create(nsize, nsize, GL_ALPHA, GL_ALPHA);
        map.texture.update(tmpBitmap);
        map.texture.generateMipmap();

        stbtt_GetScaledFontVMetrics
        (
         static_cast<SPuint8*>(m_file_data),
         0,
         static_cast<float>(charSize),
         &map.ascent,
         &map.descent,
         &map.linegap
        );

        map.linespacing = (map.ascent - map.descent + map.linegap) * map.scale;
        map.size = {nsize, nsize};
        return map;
    }

    int Font::getKerning(unsigned int first, unsigned int second)
    {
        int glyph1 = stbtt_FindGlyphIndex(static_cast<stbtt_fontinfo*>(m_font_info), first);
        int glyph2 = stbtt_FindGlyphIndex(static_cast<stbtt_fontinfo*>(m_font_info), second);
        int kerning;

        if(first == L'j') kerning = +123;
        else if(second == L'j')kerning = -123;
        else kerning = stbtt_GetGlyphKernAdvance(static_cast<stbtt_fontinfo*>(m_font_info), glyph1, glyph2);

        return kerning;
    }

    SP_Character Font::createCharacter(unsigned int codepoint, unsigned int charSize)
    {
        SP_Character character;
        if(codepoint >= 32 && codepoint < 256)
        {
            character.codepoint = codepoint;
            const SP_Font_Map& map = getMap(charSize);

            float x;
            float y;

            stbtt_fontinfo* font = static_cast<stbtt_fontinfo*>(m_font_info);
            stbtt_aligned_quad quad;
            stbtt_GetBakedQuad(reinterpret_cast<stbtt_bakedchar*>(const_cast<SP_Font_Map&>(map).char_data), map.size.x, map.size.y, codepoint - 32, &x, &y, &quad, 1);
            int advance, lsb;
            int x0, y0, x1, y1;
            stbtt_GetCodepointHMetrics(font, codepoint, &advance, &lsb);

            stbtt_GetCodepointBitmapBox(font, codepoint, map.scale, map.scale, &x0, &y0, &x1, &y1);
            character.size = {x1 - x0, y1 - y0};
            character.advance = static_cast<float>(advance) * map.scale;

            character.bounds        = rectf{quad.x0, quad.y0, quad.x1, quad.y1};
            character.tex_coords    = rectf{quad.s0, quad.t0, quad.s1, quad.t1};
            character.bearing       = {static_cast<float>(lsb), character.bounds.top};
        }

        return character;
    }

    const SP_Character& Font::getCharInfo(unsigned int codepoint, unsigned int size)
    {
        SPuint64 cipher = createCipher(codepoint, size);
        auto it = m_char_map.find(cipher);

        if(it != m_char_map.end())
        {
            return it->second;
        }
        return m_char_map.insert({cipher, createCharacter(codepoint, size)}).first->second;
    }

    const sp::Texture& Font::getTexture(unsigned charSize) const
    {
        const sp::Texture& texture = m_maps[charSize].texture;
        return texture;
    }
}

