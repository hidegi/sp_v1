#include <sp/gxsp/viewport.h>
#include <sp/sp_controller.h>
#include <sp/utils/helpers.h>
#include <sp/spgl.h>
#include <memory.h>
#include <cmath>

namespace sp
{
    namespace
    {
        static float matrix[16] =
        {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    Viewport::Viewport() :
        m_viewport  {0,0,1,1},
        m_origin    {0,0},
        m_size      {1,1},
        m_default   {true}
    {
        memcpy(m_matrix, matrix, sizeof(float) * 16);
    }

    Viewport::Viewport(const Viewport& vp) :
        m_viewport  {vp.m_viewport},
        m_origin    {vp.m_origin},
        m_size      {vp.m_size},
        m_default   {vp.m_default}
    {
        memcpy(m_matrix, vp.m_matrix, sizeof(float) * 16);
        resetViewport();
    }
    Viewport::Viewport(Viewport&& vp) :
        m_viewport  {std::move(vp.m_viewport)},
        m_origin    {std::move(vp.m_origin)},
        m_size      {std::move(vp.m_size)},
        m_default   {std::move(vp.m_default)}
    {
        vp.m_origin     = {};
        vp.m_size       = {};
        vp.m_viewport   = recti{};

        memmove(m_matrix, vp.m_matrix, sizeof(float) * 16);
        memset(vp.m_matrix, 0, sizeof(float) * 16);
        resetViewport();
    }

    Viewport::Viewport(const rectf& vp) :
        Viewport()
    {
        m_default = false;
        m_origin.x = vp.left;
        m_origin.y = vp.top;
        m_size.x = vp.width;
        m_size.y = vp.height;

        resetViewport();
    }

    Viewport::Viewport(const vec2f& pos, const vec2f& dim) :
        Viewport()
    {
        m_default = false;
        m_origin = pos;
        m_size   = dim;
        resetViewport();
    }

    void Viewport::setViewport(const rectf& rect)
    {
        m_default = false;
        m_origin.x  = rect.left;
        m_origin.y  = rect.top;
        m_size.x    = rect.width;
        m_size.y    = rect.height;
        resetViewport();
    }

    Viewport& Viewport::operator=(const Viewport& vp)
    {
        if(this != &vp)
        {
            memcpy(m_matrix, vp.m_matrix, sizeof(float) * 16);
            m_origin    = vp.m_origin;
            m_size      = vp.m_size;
            m_viewport  = vp.m_viewport;
            m_default   = vp.m_default;
            resetViewport();
        }

        return *this;
    }
    Viewport& Viewport::operator=(Viewport&& vp) noexcept
    {
        if(this != &vp)
        {
            memmove(m_matrix, vp.m_matrix, sizeof(float) * 16);
            memset(vp.m_matrix, 0, sizeof(float) * 16);
            m_origin    = vp.m_origin;
            m_size      = vp.m_size;
            m_viewport  = vp.m_viewport;
            m_default   = vp.m_default;
            vp.m_origin     = {};
            vp.m_size       = {};
            vp.m_viewport   = recti{};
            resetViewport();
        }
        return *this;
    }

    void Viewport::setOrigin(float x, float y)
    {
        m_default = false;
        m_origin.x = x;
        m_origin.y = y;
        resetViewport();
    }

    void Viewport::setOrigin(const vec2f& pos)
    {
        setOrigin(pos.x, pos.y);
    }

    void Viewport::setSize(float w, float h)
    {
        m_default = false;
        m_size.x = w;
        m_size.y = h;
        resetViewport();
    }

    void Viewport::setSize(const vec2f& dim)
    {
        setSize(dim.x, dim.y);
    }


    void Viewport::resetViewport()
    {
        float center_x = m_origin.x + m_size.x / 2.f;
        float center_y = m_origin.y + m_size.y / 2.f;

        float a =  2.f / m_size.x;
        float b = -2.f / m_size.y;
        float c = -a * center_x;
        float d = -b * center_y;

        int vpl = 0;
        int vpt = 0;
        int vpw = (int) std::floor(m_size.x + .5f);
        int vph = (int) std::floor(m_size.y + .5f);

        m_viewport = recti{vpl, vpt, vpw, vph};

        m_matrix[0] = a;
        m_matrix[5] = b;

        m_matrix[12] = c;
        m_matrix[13] = d;
    }

    void Viewport::focus(float factor)
    {
        setSize(m_size.x * factor, m_size.y * factor);
    }

    void Viewport::translate(float x, float y)
    {
        setOrigin(m_origin.x + x, m_origin.y + y);
    }

    void Viewport::translate(const vec2f& offset)
    {
        translate(offset.x, offset.y);
    }

    void Viewport::load() const
    {
        if(!Controller::active()) return;

        int top = m_size.y - (m_viewport.top + m_viewport.height);
        spCheck(glViewport(m_viewport.left, top, m_viewport.width, m_viewport.height));
        spCheck(glMatrixMode(GL_PROJECTION));
        spCheck(glLoadMatrixf(m_matrix));
        spCheck(glMatrixMode(GL_MODELVIEW));
    }

    const vec2f& Viewport::getOrigin() const
    {
        return m_origin;
    }
    const vec2f& Viewport::getSize() const
    {
        return m_size;
    }

    const float* Viewport::getMatrix() const
    {
        return m_matrix;
    }

    bool Viewport::defaulted() const
    {
        return m_default;
    }

    bool operator==(const Viewport& vp1, const Viewport& vp2)
    {
        if(vp1.defaulted() && vp2.defaulted())
            return true;

        if(vp1.defaulted() ^ vp2.defaulted())
            return false;

        return (
            !spHelperCompareFloat(vp1.m_origin.x, vp2.m_origin.x)
        &&  !spHelperCompareFloat(vp1.m_origin.y, vp2.m_origin.y)
        &&  !spHelperCompareFloat(vp1.m_size.x, vp2.m_size.x)
        &&  !spHelperCompareFloat(vp1.m_size.y, vp2.m_size.y));
    }

    bool operator!=(const Viewport& vp1, const Viewport& vp2)
    {
        return !(vp1 == vp2);
    }
}
