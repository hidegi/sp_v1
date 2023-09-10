#include <sp/arb/customsprite.h>

namespace sp
{
    CustomSprite::CustomSprite() :
        Drawable()
    {
        m_vertices.reserve(4);
        m_indices.reserve(6);

        m_indices.push_back(0);
        m_indices.push_back(1);
        m_indices.push_back(2);
        m_indices.push_back(2);
        m_indices.push_back(1);
        m_indices.push_back(3);

        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
    }

    CustomSprite::CustomSprite(const Texture& texture) :
        Drawable()
    {
        m_vertices.reserve(4);
        m_indices.reserve(6);

        m_indices.push_back(0);
        m_indices.push_back(1);
        m_indices.push_back(2);
        m_indices.push_back(2);
        m_indices.push_back(1);
        m_indices.push_back(3);

        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        setTextureSprite(texture, true);
    }

    CustomSprite::CustomSprite(const Texture& texture, const recti& area) :
        Drawable()
    {
       m_vertices.reserve(4);
        m_indices.reserve(6);

        m_update = true;

        m_indices.push_back(0);
        m_indices.push_back(1);
        m_indices.push_back(2);
        m_indices.push_back(2);
        m_indices.push_back(1);
        m_indices.push_back(3);

        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});
        m_vertices.push_back(sp::Vertex{});

        setTextureSprite(texture, true);
        setTextureRect(area);
    }

    /*
    void CustomSprite::setDrawCallback(std::function<void()> fn)
    {
        m_states.custom_draw_fn = fn;
    }
    */
    CustomSprite::Ptr CustomSprite::create()
    {
        return Ptr(new CustomSprite());
    }
    CustomSprite::Ptr CustomSprite::create(const Texture& texture)
    {
        return Ptr(new CustomSprite(texture));
    }
    CustomSprite::Ptr CustomSprite::create(const Texture& texture, const recti& area)
    {
        return Ptr(new CustomSprite(texture, area));
    }

    void CustomSprite::add(const sp::Vertex& vertex)
    {
    }

    void CustomSprite::setTexture(const sp::Texture& texture)
    {
        setTextureSprite(texture, false);
    }

    void CustomSprite::setTextureSprite(const Texture& texture, bool resetRect)
    {
        if(resetRect || (!m_states.texture && (static_cast<recti>(m_bounds) == sp::recti{})))
            setTextureRect(recti{0, 0, texture.getSize().x, texture.getSize().y});
        m_states.texture = &texture;

        m_bounds.width  = static_cast<float>(m_states.texture->getSize().x);
        m_bounds.height = static_cast<float>(m_states.texture->getSize().y);
        setSize(vec2f{m_bounds.width, m_bounds.height});
    }

    void CustomSprite::setTextureRect(const recti& area)
    {
        if(area != static_cast<recti>(m_bounds))
        {
            m_bounds = static_cast<rectf>(area);
            updatePositions();
            updateTexCoords();
            m_update = true;
        }
    }

    void CustomSprite::setSize(const vec2f size)
    {
        setSize(size.x, size.y);
    }

    void CustomSprite::setSize(float width, float height)
    {
        /*
        vec2f ratio = {width / m_bounds.width, height / m_bounds.height};
        for(size_t i = 0; i < m_vertices.size(); i++)
        {
            m_vertices[i].position.x *= ratio.x;
            m_vertices[i].position.y *= ratio.y;
        }
        */

        m_bounds.width  = width;
        m_bounds.height = height;
        updatePositions();
        m_update        = true;
    }

    void CustomSprite::updatePositions()
    {
        rectf bounds = getLocalBounds();
        m_vertices[0].position = vec2f{0.f, 0.f};
        m_vertices[1].position = vec2f{0.f, bounds.height};
        m_vertices[2].position = vec2f{bounds.width, 0.f};
        m_vertices[3].position = vec2f{bounds.width, bounds.height};
    }

    void CustomSprite::updateTexCoords()
    {
        vec2f factor;
        if(m_states.texture)
            factor    = static_cast<vec2f>(m_states.texture->getSize());
        else
            factor    = {m_bounds.width, m_bounds.height};

        float left      = m_bounds.left / factor.x;
        float right     = (left + m_bounds.width) / factor.x;
        float top       = m_bounds.top / factor.y;
        float bottom    = (top + m_bounds.height) / factor.y;

        m_vertices[0].texCoords = vec2f{left, top};
        m_vertices[1].texCoords = vec2f{left, bottom};
        m_vertices[2].texCoords = vec2f{right, top};
        m_vertices[3].texCoords = vec2f{right, bottom};
    }

    void CustomSprite::setColor(const Color& color)
    {
        m_vertices[0].color = color;
        m_vertices[1].color = color;
        m_vertices[2].color = color;
        m_vertices[3].color = color;
    }

    const Color& CustomSprite::getColor() const
    {
        return m_vertices[0].color;
    }

    void CustomSprite::setPosition(float x, float y)
    {
        Drawable::setPosition(x, y);
        updatePositions();
    }

    void CustomSprite::setPosition(const vec2f& pos)
    {
        setPosition(pos.x, pos.y);
    }


}
