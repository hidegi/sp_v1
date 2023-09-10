#include <sp/gxsp/label.h>
#include <sp/gxsp/batch_renderer.h>

namespace sp
{
    Label::Label() :
        Drawable(),
        m_map               {nullptr},
        m_string            {},
        m_previous_string   {},
        m_color             {255, 255, 255, 255},
        m_ascent            {0.f},
        m_advance           {0.f},
        m_char_size         {30},
        m_character_count   {0},
        m_previous_count    {0},
        m_traverser         {0},
        m_refresh           {true}
    {
        m_drawable_states->vertex_entry   = 0;
        m_drawable_states->index_entry    = 0;
    }

    Label::~Label()
    {
    }

    Label::Ptr Label::create()
    {
        return Ptr(new Label);
    }

    void Label::addDrawable(Renderer& renderer, bool max)
    {
        renderer.addDrawable(m_drawable_states, max);
    }
    void Label::setCharacterSize(unsigned int charSize)
    {
        //if(m_char_size == charSize)
          //  return;

        m_char_size         = charSize;
        m_map               = m_font.lock() ? &m_font.lock()->getMap(m_char_size) : nullptr;
        m_drawable_states->states.texture    = m_font.lock() ? &m_font.lock()->getTexture(m_char_size) : nullptr;
        m_refresh           = true;
        refresh();
    }

    void Label::setString(const String& text)
    {
        if(m_string == text)
            return;

        m_traverser = 0;
        m_previous_count  = m_character_count;
        m_character_count = text.length();
        m_refresh = true;
        m_string  = text;
        refresh();
    }

    void Label::setColor(const Color& color)
    {
        if(m_color == color)
            return;
        m_color = color;

        m_drawable_states->update = true;
        for(auto& v : m_vertices)
            v.color = color;
    }

    void Label::setPosition(float x, float y)
    {
        Drawable::setPosition(x, y);
        /*
        m_refresh = true;
        refresh();
        */
    }

    void Label::setFont(Font::Ptr font)
    {
        if(!font)
            return;

        m_font.reset();
        m_font = font;
        setCharacterSize(m_char_size);
    }

    void Label::setLabel(String text, Font::Ptr font, unsigned charSize, const Color& color)
    {
        setString(text);
        setFont(font);
        setCharacterSize(charSize);
        setColor(color);
    }

    void Label::insertCharacter(const vec2f& offset, const SP_Character& character)
    {
        rectf bounds =
        rectf
        {
            offset.x, offset.y,
            std::abs(character.bounds.width  - character.bounds.left) + offset.x,
            std::abs(character.bounds.height - character.bounds.top)  + offset.y,
        };

        float left   = character.tex_coords.left;
        float right  = character.tex_coords.width;
        float top    = character.tex_coords.top;
        float bottom = character.tex_coords.height;

        sp::Vertex v1 = {vec2f{bounds.left,  bounds.top},  m_color, vec2f{left, top}};
        sp::Vertex v2 = {vec2f{bounds.width, bounds.top}, m_color, vec2f{right, top}};
        sp::Vertex v3 = {vec2f{bounds.left, bounds.height}, m_color, vec2f{left, bottom}};
        sp::Vertex v4 = {vec2f{bounds.width, bounds.height}, m_color, vec2f{right, bottom}};

        add(v1); add(v2);
        add(v3); add(v3);
        add(v2); add(v4);
    }


    void Label::insertWhitespace(const vec2f& offset, float advance)
    {
        const SP_Character& character = m_font.lock()->getCharInfo(L' ', m_char_size);
        rectf bounds =
        rectf
        {
            offset.x, offset.y,
            std::abs(character.bounds.width  - character.bounds.left) + offset.x,
            std::abs(character.bounds.height - character.bounds.top)  + offset.y,
        };

        float left   = character.tex_coords.left;
        float right  = character.tex_coords.width;
        float top    = character.tex_coords.top;
        float bottom = character.tex_coords.height;

        float _left   = bounds.left;
        float _width  = advance;
        float _top    = bounds.top - offset.y;
        float _height = bounds.top;

        sp::Vertex v1 = {vec2f{_left,  _width},  m_color, vec2f{left, top}};
        sp::Vertex v2 = {vec2f{_width, _top}, m_color, vec2f{right, top}};
        sp::Vertex v3 = {vec2f{_left, _height}, m_color, vec2f{left, bottom}};
        sp::Vertex v4 = {vec2f{_width, _height}, m_color, vec2f{right, bottom}};

        add(v1); add(v2);
        add(v3); add(v3);
        add(v2); add(v4);
    }

    sp::vec2f Label::getSize() const
    {
        return sp::vec2f{m_drawable_states->bounds.width, m_drawable_states->bounds.height};
    }
    void Label::createCharacter(unsigned int prev, unsigned int curr)
    {
        if(curr == L'\r' || curr == L'\n' || curr == L'\t')
            return;

        const SP_Character& character = m_font.lock()->getCharInfo(curr, m_char_size);
        float kerning = m_map->scale * m_font.lock()->getKerning(prev, curr) * 2.f;
        float ascent = m_ascent + character.bearing.y;

        vec2f position  = m_drawable_states->position;
        vec2f offset    = vec2f{m_advance + kerning, ascent};
        if(curr != L' ')
        {
            insertCharacter(offset, character);
        }
        float distance = character.advance + kerning;
        m_advance += distance;
    }

    void Label::refresh()
    {
        if(!m_font.lock() || m_font.expired())
        {
            SP_PRINT_WARNING("cannot update string whilst having invalid font");
            return;
        }
        if(!m_map)
        {
            SP_PRINT_WARNING("cannot update string whilst having invalid map");
            return;
        }
        if(m_string.empty())
        {
            return;
        }

        /*
        m_vertices.clear();
        m_indices.clear();
        sp::vec2i size = m_map->size;
        sp::rectf bounds = rectf{0.f, 0.f, size.x, size.y};
        printf("size: %d %d\n", size.x, size.y);
        float left = 0.f;
        float top  = 0.f;
        float right = 1.f;
        float bottom = 1.f;
        sp::Vertex v1 = {vec2f{bounds.left,  bounds.top},  m_color, vec2f{left, top}};
        sp::Vertex v2 = {vec2f{bounds.width, bounds.top}, m_color, vec2f{right, top}};
        sp::Vertex v3 = {vec2f{bounds.left, bounds.height}, m_color, vec2f{left, bottom}};
        sp::Vertex v4 = {vec2f{bounds.width, bounds.height}, m_color, vec2f{right, bottom}};
        add(v1); add(v2);
        add(v3); add(v3);
        add(v2); add(v4);
        m_drawable_states->vertex_count   = m_vertices.size();
        m_drawable_states->index_count    = m_indices.size();
        m_drawable_states->update = true;
        */

        if(m_refresh)
        {
            m_vertices.clear();
            m_indices.clear();

            m_refresh = false;
            unsigned int prevChar = 0;
            m_advance = 0.f;
            m_ascent = m_font.lock()->getCharInfo(L'g', m_char_size).size.y;
            m_drawable_states->update = true;

            for(size_t i = 0; i < m_character_count; ++i)
            {
                unsigned int currentChar = m_string[i];
                createCharacter(prevChar, currentChar);
                prevChar = currentChar;
            }

            m_drawable_states->bounds.width   = m_advance;
            m_drawable_states->bounds.height  = m_ascent;

            m_drawable_states->vertex_count   = m_vertices.size();
            m_drawable_states->index_count    = m_indices.size();
        }

        //printf("texture: %d, size: %d\n", m_drawable_states->states.texture->getHandleGL(), m_char_size);
        //printf("index count: %lld, for char count: %lld\n", m_indices.size(), m_character_count);
    }
}
