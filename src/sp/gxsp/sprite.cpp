#include <sp/gxsp/sprite.h>
#include <sp/gxsp/batch_renderer.h>

namespace sp
{
    Sprite::Sprite() :
        Drawable(),
        m_custom_size{false},
        m_origin     {}
    {
        m_vertices.reserve(4);
        m_indices.reserve(6);
#if defined(SP_INITIALIZER_LIST_SUPPORTED)
        m_indices   = {0, 1, 2, 2, 1, 3};
        m_vertices  = {};
#else
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
#endif

        m_drawable_states->states.viewport = &m_viewport;
        m_drawable_states->vertex_entry = 0;
        m_drawable_states->index_entry  = 0;
        m_drawable_states->vertex_count = m_vertices.size();
        m_drawable_states->index_count  = m_indices.size();
    }

    Sprite::Sprite(const Texture& texture) :
        Drawable(),
        m_custom_size{false},
        m_origin     {}
    {
        m_vertices.reserve(4);
        m_indices.reserve(6);

#if defined(SP_INITIALIZER_LIST_SUPPORTED)
        m_indices = {0, 1, 2, 2, 1, 3};
        m_vertices  = {};
#else
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
#endif

        setTextureSprite(texture, true);
        m_drawable_states->states.viewport = &m_viewport;
        m_drawable_states->vertex_entry = 0;
        m_drawable_states->index_entry  = 0;
        m_drawable_states->vertex_count = m_vertices.size();
        m_drawable_states->index_count  = m_indices.size();
    }

    Sprite::Sprite(const Texture& texture, const recti& area) :
        Drawable(),
        m_custom_size{false},
        m_origin     {}
    {
        m_vertices.reserve(4);
        m_indices.reserve(6);

        m_drawable_states->update = true;

#if defined(SP_INITIALIZER_LIST_SUPPORTED)
        m_indices = {0, 1, 2, 2, 1, 3};
        m_vertices  = {};
#else
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
#endif

        setTextureSprite(texture, true);
        setTextureRect(area);
        m_drawable_states->states.viewport = &m_viewport;
        m_drawable_states->vertex_entry = 0;
        m_drawable_states->index_entry  = 0;
        m_drawable_states->vertex_count = m_vertices.size();
        m_drawable_states->index_count  = m_indices.size();
    }


    Sprite::~Sprite()
    {
    }

    void Sprite::setOrigin(float x, float y)
    {
        if(!m_drawable_states->bounds.contains(x, y))
            return;
        m_origin.x = x;
        m_origin.y = y;
    }
    void Sprite::setOrigin(const vec2f& orig)
    {
        setOrigin(orig.x, orig.y);
    }

    const vec2f& Sprite::getOrigin() const
    {
        return m_origin;
    }

    Sprite::Ptr Sprite::create()
    {
        return Ptr(new Sprite());
    }
    Sprite::Ptr Sprite::create(const Texture& texture)
    {
        return Ptr(new Sprite(texture));
    }
    Sprite::Ptr Sprite::create(const Texture& texture, const recti& area)
    {
        return Ptr(new Sprite(texture, area));
    }

    void Sprite::add(const sp::Vertex& vertex)
    {
    }

    void Sprite::setTexture(const sp::Texture& texture)
    {
        setTextureSprite(texture, false);
    }

    void Sprite::setTextureSprite(const Texture& texture, bool resetRect)
    {
        m_drawable_states->states.texture = &texture;
        if(resetRect || (static_cast<recti>(m_drawable_states->bounds) == sp::recti{}))
        {
            setTextureRect(recti{0, 0, texture.getSize().x, texture.getSize().y});
        }

        m_drawable_states->bounds.width  = static_cast<float>(m_drawable_states->states.texture->getSize().x);
        m_drawable_states->bounds.height = static_cast<float>(m_drawable_states->states.texture->getSize().y);
        //setSize(vec2f{m_drawable_states->bounds.width, m_drawable_states->bounds.height});
    }

    void Sprite::setTextureRect(const recti& area)
    {
        if(area != static_cast<recti>(m_drawable_states->bounds))
        {
            m_drawable_states->bounds = static_cast<rectf>(area);
            if(!m_custom_size)
                updatePositions();
            updateTexCoords();
            m_drawable_states->update = true;
        }
    }

    void Sprite::setSize(const vec2f size)
    {
        setSize(size.x, size.y);
    }

    void Sprite::addDrawable(Renderer& renderer, bool max)
    {
        renderer.addDrawable(m_drawable_states, max);
    }
    void Sprite::setSize(float width, float height)
    {
        /*
        vec2f ratio = {width / m_drawable_states->bounds.width, height / m_drawable_states->bounds.height};
        for(size_t i = 0; i < m_vertices.size(); i++)
        {
            m_vertices[i].position.x *= ratio.x;
            m_vertices[i].position.y *= ratio.y;
        }
        */
        /*
        if(m_drawable_states->states.texture)
            m_drawable_states->states.texture->generateMipmap();
        */
        m_drawable_states->bounds.width  = width;
        m_drawable_states->bounds.height = height;
        updatePositions();
        m_custom_size                    = true;
        m_drawable_states->update        = true;
    }

    void Sprite::setScale(float x, float y)
    {
        setSize(x * m_drawable_states->bounds.width, y * m_drawable_states->bounds.height);
    }

    void Sprite::setScale(const vec2f& scale)
    {
        setScale(scale.x, scale.y);
    }

    void Sprite::updatePositions()
    {
        rectf bounds = getLocalBounds();
        m_vertices[0].position = vec2f{0.f, 0.f};
        m_vertices[1].position = vec2f{0.f, bounds.height};
        m_vertices[2].position = vec2f{bounds.width, 0.f};
        m_vertices[3].position = vec2f{bounds.width, bounds.height};
    }

    void Sprite::updateTexCoords()
    {
        /*
        vec2f factor;

        if(m_drawable_states->states.texture)
            factor    = static_cast<vec2f>(m_drawable_states->states.texture->getSize());
        else
            factor    = {m_drawable_states->bounds.width, m_drawable_states->bounds.height};

        float left      = m_drawable_states->bounds.left / factor.x;
        float right     = (left + m_drawable_states->bounds.width) / factor.x;
        float top       = m_drawable_states->bounds.top / factor.y;
        float bottom    = (top + m_drawable_states->bounds.height) / factor.y;
        */
        const sp::Texture* texture = m_drawable_states->states.texture;
        vec2f size = texture ? (static_cast<vec2f>(texture->getSize())) : vec2f{1.f, 1.f};

        float left      = m_drawable_states->bounds.left / size.x;
        float top       = m_drawable_states->bounds.top / size.y;
        float right     = (m_drawable_states->bounds.left + m_drawable_states->bounds.width) / size.x;
        float bottom    = (m_drawable_states->bounds.top + m_drawable_states->bounds.height) / size.y;

        //printf("tex coords: %f %f %f %f, bounds: %f %f %f %f\n", left * size.x, top * size.y, right * size.x, bottom * size.y,
           //    m_drawable_states->bounds.left, m_drawable_states->bounds.top, m_drawable_states->bounds.width, m_drawable_states->bounds.height);
        m_vertices[0].texCoords = vec2f{left, top};
        m_vertices[1].texCoords = vec2f{left, bottom};
        m_vertices[2].texCoords = vec2f{right, top};
        m_vertices[3].texCoords = vec2f{right, bottom};
        m_drawable_states->update = true;
    }

    void Sprite::setColor(const Color& color)
    {
        m_vertices[0].color = color;
        m_vertices[1].color = color;
        m_vertices[2].color = color;
        m_vertices[3].color = color;
        m_drawable_states->update = true;
    }

    const Color& Sprite::getColor() const
    {
        return m_vertices[0].color;
    }

    void Sprite::setPosition(float x, float y)
    {
        Drawable::setPosition(x - m_origin.x, y - m_origin.y);
        //updatePositions();
    }

    void Sprite::setPosition(const vec2f& pos)
    {
        setPosition(pos.x, pos.y);
    }

    void Sprite::setViewport(const rectf& viewport)
    {
        m_viewport.setViewport(viewport);
    }
    const Viewport& Sprite::getViewport() const
    {
        return m_viewport;
    }
}
