#include <sp/lxsp/light_source.h>

namespace sp
{
    LightSource::LightSource() :
        Drawable(),
        m_color {255, 255, 255, 255},
        m_fade  {true}
    {
    }

    LightSource::~LightSource()
    {

    }

    void LightSource::setIntensity(float intensity)
    {
        m_color.a = 255 * intensity;
        resetColor();
    }

    float LightSource::getIntensity() const
    {
        return (float) m_color.a / 255.f;
    }

    void LightSource::setColor(const Color& c)
    {
        m_color = c;
        resetColor();
    }

    Color LightSource::getColor() const
    {
        const Color& c = m_color;
        return Color{c.r, c.g, c.b, 255};
    }

    void LightSource::setFade(bool fade)
    {
        m_fade = fade;
        resetColor();
    }

    bool LightSource::hasFade() const
    {
        return m_fade;
    }

    void LightSource::setRange(float r)
    {
        m_range = r;
    }

    float LightSource::getRange() const
    {
        return m_range;
    }
}
