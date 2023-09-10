#include <sp/arb/overlay.h>
#include <sp/gxsp/batch_renderer.h>

namespace sp
{
    SpriteBatch::SpriteBatch() :
        m_origin{0.f, 0.f}
    {
    }

    SpriteBatch::Ptr SpriteBatch::create()
    {
        return Ptr(new SpriteBatch);
    }

    void SpriteBatch::setPosition(int index, const sp::vec2f& pos)
    {
        if(index < 0 || index >= m_sprites.size())
            return;

        m_sprites[index].setPosition(pos);
        /*
        sp::vec2f origin = (m_vertices.begin() + index * 4)->position;

        for(auto it = m_vertices.begin() + index * 4; it != m_vertices.begin() + index * 4 + 4; it++)
        {
            sp::vec2f offset = it->position - origin;
            it->position = pos + offset;
        }
        m_drawable_states->update = true;
        */
    }
    void SpriteBatch::setColor(int index, const sp::Color& color)
    {
        if(index < 0 || index >= m_sprites.size())
            return;
        //m_sprites[index].setColor(color);
        m_sprites[index].m_drawable_states->update = true;
        for(auto it = m_vertices.begin() + index * 4; it != m_vertices.begin() + index * 4 + 4; it++)
            it->color = color;
        m_drawable_states->update = true;
    }

    void SpriteBatch::setVisible(int index, bool visible)
    {
        if(index < 0 || index >= m_sprites.size())
            return;
        m_sprites[index].setVisible(visible);
    }

    size_t SpriteBatch::size() const
    {
        return m_sprites.size();
    }

    int SpriteBatch::addQuad(const Texture& texture, const vec2f& pos, const vec2f& size, const recti& area)
    {
        int return_index = m_sprites.size();
        m_sprites.push_back(Sprite(texture));
        Sprite& sprite = m_sprites.back();
        //sprite.setTextureSprite(texture);

        sp::vec2f sz = size;
        sp::recti ar = area;

        if(sz == sp::vec2f{})
            sz = texture.getSize();

        if(area == sp::recti{})
            ar = sp::recti{{0, 0}, texture.getSize()};

        sprite.setSize(sz);
        sprite.setTextureRect(ar);

        rectf bounds = sprite.m_drawable_states->bounds;

        sprite.m_drawable_states->vertex_entry    = m_vertices.size();
        sprite.m_drawable_states->index_entry     = m_indices.size();
        sprite.m_drawable_states->position        = pos;
        sprite.m_drawable_states->update          = true;
        //sprite.m_drawable_states->id              = ++generator;

        sprite.m_drawable_states->client          = this;
        sprite.m_drawable_states->states.viewport = nullptr;
        sprite.m_drawable_states->states.texture  = &texture;

        m_vertices.insert(m_vertices.end(), sprite.m_vertices.begin(), sprite.m_vertices.end());
        m_indices.insert(m_indices.end(), sprite.m_indices.begin(), sprite.m_indices.end());

        return return_index;
    }

    void SpriteBatch::addDrawable(Renderer& renderer, bool max)
    {
        for(Sprite& sprite: m_sprites)
        {
            renderer.addDrawable(sprite.m_drawable_states, max);
        }
    }

    void SpriteBatch::removeDrawable(Renderer& renderer)
    {
         for(Sprite& sprite : m_sprites)
         {
             renderer.removeDrawable(sprite.m_drawable_states);
         }
    }

    bool SpriteBatch::isVisible(int index) const
    {
        if(index < 0 || index >= m_sprites.size())
            return false;

        return m_sprites[index].isVisible();
    }

    void SpriteBatch::removeSprite(int index)
    {
        if(index < 0 || index >= m_sprites.size())
            return;

        m_sprites.erase(m_sprites.begin() + index);
    }



    void SpriteBatch::uniformTranslate(const vec2f& pos)
    {
        vec2f prev_origin = m_origin;
        m_origin = pos;
        for(auto& sprite : m_sprites)
        {
            vec2f offset = sprite.m_drawable_states->position - prev_origin;
            sprite.setPosition(pos + offset);
        }
    }

    void SpriteBatch::uniformVisibility(bool visible)
    {
        for(auto& sprite : m_sprites)
        {
            sprite.setVisible(visible);
        }
    }
    void SpriteBatch::uniformTranslate(float x, float y)
    {
        uniformTranslate(vec2f{x, y});
    }


    const Sprite& SpriteBatch::get(int index) const
    {
        if(index < 0 || index >= m_sprites.size())
            return sp::Sprite{};

        return m_sprites[index];
    }

    Sprite& SpriteBatch::get(int index)
    {
        if(index < 0 || index >= m_sprites.size())
        {
            static sp::Sprite sprite;
            return sprite;
        }
        return m_sprites[index];
    }
}
