#include <sp/arb/map2d.h>
#include <sp/arb/levels.h>
#include <sp/gxsp/framebuffer.h>

namespace sp
{
    namespace
    {
        sp::Framebuffer framebuffer;
    }

    Map2D::Map2D()  :
        m_bounds            {},
        m_viewport          {},
        m_texture_atlas     {nullptr},
        m_output_sprite     {nullptr},
        m_grid              {},
        m_padding           {0},
        m_unit_length       {0},
        m_grid_texture      {},
        m_rule_set          {}
    {
    }
    sp::Map2D::Tile Map2D::parseDelimiter(sp::String str, int& _id)
    {
        sp::Map2D::Tile tile;
        tile.walkable = false;

        size_t pos = str.find_first_of(":;");
        unsigned int closing = 0;
        size_t start_f = 0;
        size_t start = 0;
        size_t end   = 0;

        int id = -1;
        while(pos != sp::String::npos)
        {
            if(str[pos] == L':')
            {
                start = pos;
                if(end)
                    start_f = end + 1;
                ++closing;
            }
            if(str[pos] == L';')
            {
                end = pos;
                --closing;
            }

            if(!closing)
            {
                sp::String token = str.substr(start_f , start - start_f);
                sp::String value = str.substr(start + 1, end - start - 1);

                if(token == "id")
                {
                    id = std::stoi(value.toStdString().c_str());
                    //tiles.insert({id, Tile{}});
                    _id = id;
                }

                if(token == "padding")
                {
                    m_padding = std::stoi(value.toStdString().c_str());
                }

                if(token == "unit")
                {
                    m_unit_length = std::stoi(value.toStdString().c_str());
                }

                if(token == "walkable")
                {
                    bool w = (value == "true");
                    tile.walkable = w;
                }

                if(token == "x")
                {

                    int x = std::stoi(value.toStdString().c_str());
                    tile.tex_coords.x = x;
                }

                if(token == "y")
                {
                    int y = std::stoi(value.toStdString().c_str());
                    tile.tex_coords.y = y;
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

                    //printf("%s: %s;", sub1.toStdString().c_str(), sub.toStdString().c_str());
            }

            pos = str.find_first_of(":;", pos + 1);
        }
        return tile;
    }

    void Map2D::loadRuleSet(const char* path)
    {
        size_t file_length;
        std::u32string str;
        char* data = reinterpret_cast<char*>(spHelperFileData(path, &file_length));
        sp::String output(str);

        for(size_t i = 0; i < file_length; i++)
        {
            if(isWhitespace(data[i]))
                continue;
            output.append(data[i]);
        }
        //output.trim();
        size_t pos   = output.find_first_of("{}");
        size_t start = 0;
        size_t end   = 0;
        unsigned int brace_count = 0;
        sp::String::const_iterator c = output.begin();

        while(pos != sp::String::npos)
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
                int id = -1;
                Tile tile = parseDelimiter(sub, id);
                m_rule_set.insert({id, tile});
            }

            pos = output.find_first_of("{}", pos + 1);
        }

        //printf("%s", output.toStdString().c_str());

        free(data);
    }

    sp::Sprite::Ptr Map2D::getSprite() const
    {
        return m_output_sprite;
    }

    void Map2D::setTextureAtlas(const Texture& texture)
    {
        m_texture_atlas = &texture;
    }
    void Map2D::loadLevel(const void* data)
    {
        if(m_rule_set.empty())
        {
            SP_PRINT_WARNING("no rule set has been specified");
            return;
        }

        if(!m_unit_length)
        {
            SP_PRINT_WARNING("unit length cannot be set to zero");
            return;
        }

        static Vertex texture_quad[4];
        texture_quad[0].position = vec2f{0.f, 0.f};
        texture_quad[1].position = vec2f{0.f, m_unit_length};
        texture_quad[2].position = vec2f{m_unit_length, 0.f};
        texture_quad[3].position = vec2f{m_unit_length, m_unit_length};

        const unsigned char* tiles = reinterpret_cast<const unsigned char*>(data);

        unsigned int width  = m_dimensions.x;
        unsigned int height = m_dimensions.y;

        m_grid.reserve(width * height);
        printf("width: %d height: %d\n", width, height);
        framebuffer.create(width * m_unit_length, height * m_unit_length);
        framebuffer.bind();
        for(size_t i = 0; i < width * height; i++)
        {
            //float pos_x = (float)(static_cast<int>(i / width) * m_unit_length + m_padding);
            //float pos_y = (float)(width * height/ (i + 1));

            float pos_y = (float)(i / m_dimensions.x) * m_unit_length;
            float pos_x = (float)(i % m_dimensions.x) * m_unit_length;
            unsigned int token = static_cast<int>(tiles[i]);

            auto it = m_rule_set.find(token);
            if(it == m_rule_set.end())
                continue;

            Tile& tile = it->second;
            m_grid.push_back(tile.walkable);
            unsigned int x = tile.tex_coords.x * 64 + tile.tex_coords.x * m_padding + 1;
            unsigned int y = tile.tex_coords.y * 64 + tile.tex_coords.y * m_padding + 1;

            vec2f top_left      = {float(x), float(y)};
            /*
            texture_quad[0].texCoords = vec2f{top_left.x * 64 + m_padding, top_left.y * 64 + m_padding};
            texture_quad[1].texCoords = vec2f{top_left.x * 64 + m_padding, 64};
            texture_quad[2].texCoords = vec2f{64, top_left.y * 64 + m_padding};
            texture_quad[3].texCoords = vec2f{64, 64};
            */

            texture_quad[0].texCoords = vec2f{top_left.x,        top_left.y};
            texture_quad[1].texCoords = vec2f{top_left.x,        top_left.y + 64.f};
            texture_quad[2].texCoords = vec2f{top_left.x + 64.f, top_left.y};
            texture_quad[3].texCoords = vec2f{top_left.x + 64.f, top_left.y + 64};
            framebuffer.draw(texture_quad, 4, TriangleStrip,
                             vec2f{pos_x, pos_y},
                             vec2f{1.f, 1.f},
                             0.f,
                             BlendAlpha,
                             m_texture_atlas,
                             nullptr
                             );
        }

        framebuffer.display();

        m_grid_texture.create(width * m_unit_length, height * m_unit_length);
        m_grid_texture.setFlipped(true);
        m_grid_texture.update(framebuffer.getColorTexture());

        framebuffer.destroy();

        m_output_sprite = sp::Sprite::create();
        m_output_sprite->setTexture(m_grid_texture);
        m_output_sprite->setColor(Color{0, 0, 255});

        m_bounds = rectf{0.f, 0.f, m_dimensions.x * 16.f, m_dimensions.y * 16.f};

        //m_output_sprite->setScale(2.f, 2.f);
    }

    const sp::Texture& Map2D::getLevelTexture() const
    {
        return m_grid_texture;
    }


    vec2f Map2D::mapPixels(const vec2f& external)
    {
        vec2f normalized = vec2f {-1.f + 2.f * external.x / m_bounds.width,
                                   1.f - 2.f * external.y / m_bounds.height};

        /*
        static Transformable trm;
        trm.setPosition(0, 0);
        trm.set
        */
    }
}
