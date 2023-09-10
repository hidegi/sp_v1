#include <sp/gxsp/transformable.h>
#include <cmath>

namespace sp
{
    Transformable::Transformable() :
        m_origin        {0, 0},
        m_position      {0, 0},
        m_rotation      {0},
        m_scale         {1, 1},
        m_matrix        {},
        m_update_mat    {true},
        m_inv_matrix    {},
        m_update_inv_mat{true}
    {
    }

    Transformable::~Transformable()
    {
    }

    void Transformable::setPosition(float x, float y)
    {
        m_position.x = x;
        m_position.y = y;
        m_update_mat = m_update_inv_mat = true;
    }

    void Transformable::setPosition(const vec2f& pos)
    {
        setPosition(pos.x, pos.y);
    }

    void Transformable::setRotation(float angle)
    {
        m_rotation = static_cast<float>(fmod(angle, 360));
        if(m_rotation < 0)
            m_rotation += 360.f;
        m_update_mat = m_update_inv_mat = true;
    }

    void Transformable::setScale(float fx, float fy)
    {
        m_scale.x = fx;
        m_scale.y = fy;
        m_update_mat = m_update_inv_mat = true;
    }

    void Transformable::setScale(const vec2f& scale)
    {
        setScale(scale.x, scale.y);
    }

    void Transformable::setOrigin(float x, float y)
    {
        m_origin.x = x;
        m_origin.y = y;
        m_update_mat = m_update_inv_mat = true;
    }

    void Transformable::setOrigin(const vec2f& orig)
    {
        setOrigin(orig.x, orig.y);
    }

    const vec2f& Transformable::getPosition() const
    {
        return m_position;
    }
    const vec2f& Transformable::getScale() const
    {
        return m_scale;
    }

    float Transformable::getRotation() const
    {
        return m_rotation;
    }

    const vec2f& Transformable::getOrigin() const
    {
        return m_origin;
    }

    void Transformable::move(float x, float y)
    {
        setPosition(m_position.x + x, m_position.y + y);
    }

    void Transformable::move(const vec2f& offset)
    {
        setPosition(m_position.x + offset.x, m_position.y + offset.y);
    }

    void Transformable::rotate(float angle)
    {
        setRotation(m_rotation + angle);
    }

    void Transformable::scale(float sx, float sy)
    {
        setScale(m_scale.x * sx, m_scale.y * sy);
    }

    void Transformable::scale(const vec2f& scale)
    {
        setScale(m_scale.x * scale.x, m_scale.y * scale.y);
    }

    const mat& Transformable::getMatrix() const
    {
        if(m_update_mat)
        {
            float angle = -m_rotation * 3.141592654f / 180.f;
            float cos   = static_cast<float>(std::cos(angle));
            float sin   = static_cast<float>(std::sin(angle));
            float sxc   = m_scale.x * cos;
            float syc   = m_scale.y * cos;
            float sxs   = m_scale.x * sin;
            float sys   = m_scale.y * sin;
            float tx    = -m_origin.x * sxc - m_origin.y * sys + m_position.x;
            float ty    =  m_origin.x * sxs - m_origin.y * syc + m_position.y;

            m_matrix = mat(sxc, sys, tx,
                           -sxs, syc, ty,
                           0.f, 0.f, 1.f);
            m_update_mat = false;
        }

        return m_matrix;
    }

    const mat& Transformable::getInverseMatrix() const
    {
        if(m_update_inv_mat)
        {
            m_update_inv_mat = false;
            m_inv_matrix = mat(!m_matrix);
        }

        return m_inv_matrix;
    }
}
