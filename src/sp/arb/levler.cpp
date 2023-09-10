#include <sp/arb/levler.h>
#include <sp/gxsp/framebuffer.h>

namespace sp
{
    namespace
    {
        Framebuffer framebuffer;
    }

    Levler::Levler()  :
        m_texture_atlas     {nullptr},
        m_sprite_unit_length{16},
        m_padding           {0},
        m_tex_unit_length   {0},
        m_output_texture    {},
        m_rule_set          {},
        m_overlay_count     {0},
        m_sprite_batch      {nullptr},
        m_output_sprite     {nullptr}
    {
    }

    Levler::~Levler()
    {
        printf("levler died..\n");
    }

    void Levler::parseSection(const String& ex)
    {
        Evaluator evaluator;
        evaluator.walkable = false;
        evaluator.empty    = false;
        evaluator.overlay  = false;

        size_t pos = ex.find_first_of(";:");
        unsigned int closing = 0;
        size_t colon = 0;
        size_t start = 0;
        size_t end   = 0;

        int id = -1;

        while(pos != sp::String::npos)
        {
            if(ex[pos] == L':')
            {
                start = pos;
                if(end)
                    colon = end + 1;
                ++closing;
            }

            if(ex[pos] == L';')
            {
                end = pos;
                --closing;
            }

            if(!closing)
            {
                sp::String token = ex.substr(colon, start - colon);
                sp::String value = ex.substr(start + 1, end - start - 1);

                if(token == "id")
                {
                    id = std::stoi(value.toStdString().c_str());
                }

                if(token == "padding")
                {
                    m_padding = std::stoi(value.toStdString().c_str());
                }

                if(token == "unit")
                {
                    m_tex_unit_length = std::stoi(value.toStdString().c_str());
                }

                if(token == "walkable")
                {
                    evaluator.walkable = (value == "true");
                }
                if(token == "x")
                {

                    int x = std::stoi(value.toStdString().c_str());
                    evaluator.texCoords.x = x;
                }

                if(token == "y")
                {
                    int y = std::stoi(value.toStdString().c_str());
                    evaluator.texCoords.y = y;
                }

                if(token == "width")
                {

                    int x = std::stoi(value.toStdString().c_str());
                    m_dimensions.x = x;
                }

                if(token == "height")
                {
                    int y = std::stoi(value.toStdString().c_str());
                    m_dimensions.y = y;
                }

                if(token == "empty")
                {
                    evaluator.empty = (value == "true");
                }

                if(token == "overlay")
                {
                    evaluator.overlay = (value == "true");

                }
            }
            pos = ex.find_first_of(":;", pos + 1);
        }

        if(id != -1)
            m_rule_set.insert({id, evaluator});
    }

    bool Levler::isOutOfBounds(const sp::vec2f& pos)
    {
        return isOutOfBounds(pos.x, pos.y);
    }

    bool Levler::isOutOfBounds(float x, float y)
    {
        return (     x < m_bounds.left || x > (m_bounds.left + m_bounds.width)
                ||   y < m_bounds.top  || y > (m_bounds.top  + m_bounds.height));
    }

    void Levler::loadRuleSet(const char* path)
    {
        sp::String output;
        size_t file_length;
        char* data = reinterpret_cast<char*>(spHelperFileData(path, &file_length));
        for(size_t i = 0; i < file_length; i++)
        {
            if(isWhitespace(data[i]))
                continue;
            output.append(data[i]);
        }

        size_t pos = output.find_first_of("{}");
        size_t start = 0;
        size_t end = 0;
        unsigned int brace_count = 0;
        while(pos != String::npos)
        {
            if(output[pos] == L'{')
            {
                start = pos;
                ++brace_count;
            }

            if(output[pos] == L'}')
            {
                end = pos;
                --brace_count;
            }

            if(brace_count < 0)
            {
                printf("parse error..\n");
                break;
            }

            if(!brace_count)
            {
                sp::String sub = output.substr(start + 1, pos - start - 1);
                parseSection(sub);
            }

            pos = output.find_first_of("{}", pos + 1);
        }

        free(data);
    }

    vec2u Levler::getMapSize() const
    {
        return m_dimensions;
    }
    unsigned int Levler::getDrawableUnitLength()
    {
        return m_sprite_unit_length;
    }
    void Levler::setDrawableUnitLength(unsigned int length)
    {
        m_sprite_unit_length = length;
    }

    void Levler::setSourceTexture(const Texture& texture)
    {
        m_texture_atlas = &texture;
    }

    Texture Levler::getOutputTexture()
    {
        return m_output_texture;
    }

    vec2f Levler::getOrigin() const
    {
        return vec2f{m_bounds.left, m_bounds.top};
    }
    static int ind = 0;
    void Levler::loadLevel(const void* data)
    {
        if(m_rule_set.empty())
        {
            SP_PRINT_WARNING("no rule set has been specified");
            return;
        }

        if(!m_tex_unit_length)
        {
            SP_PRINT_WARNING("unit length cannot be set to zero");
            return;
        }

        if(!m_texture_atlas)
        {
            SP_PRINT_WARNING("no texture atlas specified");
        }

        if(m_dimensions == vec2u{})
        {
            SP_PRINT_WARNING("dimensions cannot be set to zero");
            return;
        }
        m_grid.clear();

        m_sprite_batch  = SpriteBatch::create();
        m_output_sprite = Sprite::create();

        const unsigned char* tiles = reinterpret_cast<const unsigned char*>(data);

        unsigned int width  = m_dimensions.x;
        unsigned int height = m_dimensions.y;
        m_grid.reserve(width * height);

        framebuffer.destroy();
        framebuffer.create(width * m_sprite_unit_length, height * m_sprite_unit_length);
        framebuffer.bind();
        framebuffer.clear();

        unsigned int unit_length = m_tex_unit_length + m_padding + 1;

        for(size_t i = 0; i < width * height; i++)
        {
            float pos_y = (float)(i / m_dimensions.x) * m_sprite_unit_length;
            float pos_x = (float)(i % m_dimensions.x) * m_sprite_unit_length;
            unsigned int token = static_cast<int>(tiles[i]);

            auto it = m_rule_set.find(token);
            if(it == m_rule_set.end())
            {
                //m_grid.push_back(tile);
                continue;
            }

            Evaluator& evaluator = it->second;

            Tile tile;
            tile.id       = token;
            tile.walkable = evaluator.walkable;
            tile.index    = -1;

            if(evaluator.empty)
            {
                m_grid.push_back(tile);
                continue;
            }

            unsigned int x = evaluator.texCoords.x * m_tex_unit_length + evaluator.texCoords.x * m_padding + m_padding;
            unsigned int y = evaluator.texCoords.y * m_tex_unit_length + evaluator.texCoords.y * m_padding + m_padding;
            vec2f top_left      = {float(x), float(y)};

            static int motex = 0;
            if(evaluator.overlay)
            {
                tile.index = m_sprite_batch->addQuad(*m_texture_atlas, vec2f{pos_x, pos_y}, vec2f{m_sprite_unit_length, m_sprite_unit_length}, recti{x, y, m_tex_unit_length, m_tex_unit_length});
                ++m_overlay_count;
            }
            else
            {
                static Vertex texture_quad[4];
                texture_quad[0].position = vec2f{0.f, 0.f};
                texture_quad[1].position = vec2f{0.f, m_sprite_unit_length};
                texture_quad[2].position = vec2f{m_sprite_unit_length, 0.f};
                texture_quad[3].position = vec2f{m_sprite_unit_length, m_sprite_unit_length};



                texture_quad[0].texCoords = vec2f{top_left.x,        top_left.y};
                texture_quad[1].texCoords = vec2f{top_left.x,        top_left.y + m_tex_unit_length};
                texture_quad[2].texCoords = vec2f{top_left.x + m_tex_unit_length, top_left.y};
                texture_quad[3].texCoords = vec2f{top_left.x + m_tex_unit_length, top_left.y + m_tex_unit_length};

                framebuffer.draw(texture_quad, 4, TriangleStrip,
                                 vec2f{pos_x, pos_y},
                                 vec2f{1.f, 1.f},
                                 0.f,
                                 BlendAlpha,
                                 m_texture_atlas,
                                 nullptr
                                 );
            }
             m_grid.push_back(tile);
        }
        framebuffer.display();
        m_output_texture.create(width * m_sprite_unit_length, height * m_sprite_unit_length);
        m_output_texture.setFlipped(true);
        m_output_texture.update(framebuffer.getColorTexture());

        framebuffer.bind();
        framebuffer.clear();
        framebuffer.display();

        m_output_sprite->setTexture(m_output_texture);
        m_output_sprite->setSize(m_dimensions.x * m_sprite_unit_length, m_dimensions.y * m_sprite_unit_length);
        setViewport(sp::rectf{0, 0, m_dimensions.x * m_sprite_unit_length, m_dimensions.y * m_sprite_unit_length});
    }

    sp::Sprite::Ptr Levler::getOutputSprite()
    {
        return m_output_sprite;
    }

    unsigned int Levler::getOverlayDrawableCount() const
    {
        return m_overlay_count;
    }

    void Levler::setKeyPosition(const sp::vec2i& pos, Predicate predicate)
    {
        if(m_grid.empty())
        {
            SP_PRINT_WARNING("cannot specify key positions without known the grid");
            return;
        }
        if(!predicate)
            return;

        sp::vec2u map_size = getMapSize();
        int index = pos.y * map_size.x + pos.x;
        m_key_positions.insert({index, predicate});
        //m_key_positions.push_back(pos);
    }

    void Levler::setViewport(const sp::rectf& vp)
    {
        m_bounds = vp;
        m_sprite_batch->uniformTranslate(vp.left, vp.top);
    }

    vec2f Levler::mapToViewport(float x, float y)
    {
        sp::vec2f ndc = mapPixels(x, y);
        sp::vec2f out = {(ndc.x + 1.f) * .5f * m_bounds.width, (1.f - ndc.y) * .5f * m_bounds.height};
        return out;
    }
    vec2f Levler::mapToViewport(const vec2f& pos)
    {
        return mapToViewport(pos.x, pos.y);
    }

    vec2f Levler::mapPixels(float x, float y)
    {
        sp::vec2f normalized;
        normalized.x = -1.f + 2.f * (x - m_bounds.left) / m_bounds.width;
        normalized.y =  1.f - 2.f * (y - m_bounds.top) / m_bounds.height;
        m_transformable.setPosition(m_bounds.left, m_bounds.top);
        return m_transformable.getInverseMatrix().transformVertex(normalized);
    }

    vec2f Levler::mapPixels(const vec2f& pos)
    {
        return mapPixels(pos.x, pos.y);
    }

    vec2i Levler::mapCoords(float x, float y)
    {
        m_transformable.setPosition(0.f, 0.f);
        sp::vec2f normalized = m_transformable.getMatrix().transformVertex(x, y);

        sp::vec2i pixel;
        pixel.x = static_cast<int>(std::floor(( normalized.x + 1.f) / 2.f * m_bounds.width + m_bounds.left + .5f));
        pixel.y = static_cast<int>(std::floor((-normalized.y + 1.f) / 2.f * m_bounds.height + m_bounds.top + .5f));
        return pixel;
    }

    vec2i Levler::mapCoords(const vec2f& pos)
    {
        return mapCoords(pos.x, pos.y);
    }

    bool Levler::walkable(const vec2f& pos)
    {
        return walkable(pos.x, pos.y);
    }

    vec2i Levler::mapToGrid(float x, float y)
    {
        int width   = (int)(std::floor(m_bounds.width  / m_sprite_unit_length + .5f));
        int height  = (int)(std::floor(m_bounds.height / m_sprite_unit_length + .5f));

        int _x = /*sp::clamp(*/(int)(std::floor((x - m_sprite_unit_length / 2.f) / m_sprite_unit_length + .5f )); //, 0, width) ;
        int _y = /*sp::clamp(*/(int)(std::floor((y - m_sprite_unit_length / 2.f) / m_sprite_unit_length + .5f )); //, 0, height);
        return vec2i{_x, _y};
    }
    vec2i Levler::mapToGrid(const vec2f& pos)
    {
        return mapToGrid(pos.x, pos.y);
    }
    bool Levler::walkable(float x, float y)
    {
        return getTile(x, y).walkable;
    }

    int Levler::getIndex(const vec2f& pos)
    {
        return getIndex(pos.x, pos.y);
    }

    int Levler::getIndex(float x, float y)
    {
        vec2f local = mapToViewport(x, y);
        vec2i pos   = mapToGrid(local.x, local.y);
        int _x = pos.x;
        int _y = pos.y;
        int width   = (int)(std::floor(m_bounds.width  / m_sprite_unit_length + .5f));
        int index = _y * width + _x;

        if(index >= m_grid.size() ||index < 0)
            return -1;

        return index;
    }


    bool Levler::isVisible(int index) const
    {
        if(!((index < m_grid.size()) && (index >= 0) && (m_grid[index].index != -1)))
            return false;

        return m_sprite_batch->isVisible(m_grid[index].index);
    }

    void Levler::setVisible(int index, bool visible)
    {
        if((index < m_grid.size()) && (index >= 0) && (m_grid[index].index != -1))
        {
            m_sprite_batch->setVisible(m_grid[index].index, visible);
        }
    }

    const Tile& Levler::getTile(const sp::vec2f& pos)
    {
        return getTile(pos.x, pos.y);
    }

    const Tile& Levler::getTile(float x, float y)
    {
        static Tile tile = { -1, -1, false };
        if(isOutOfBounds(x, y))
            return tile;

        int index = getIndex(x, y);

        if(index == -1)
            return tile;

        return m_grid[index];
    }

    sp::SpriteBatch::Ptr Levler::getSpriteBatch()
    {
        return m_sprite_batch;
    }
}
