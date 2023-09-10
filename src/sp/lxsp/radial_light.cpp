#include <sp/lxsp/radial_light.h>
#include <sp/gxsp/vertex_array.h>
#include <sp/gxsp/transformable.h>
#include <algorithm>
#define PI 3.141592654

namespace sp
{
    namespace
    {
        //lock these all..
        Framebuffer texture_fade;
        Framebuffer texture_plain;

        bool        texture_ready;
        const float BASE_RADIUS = 400.f;
    }

    int RadialLight::instance_count = 0;
    sp::Texture RadialLight::fade_texture;

    RadialLight::RadialLight() :
        LightSource()
    {

        if(!texture_ready)
        {
            texture_ready = true;
            initializeTextures();
        }
        /*
        m_polygon.setType(TriangleFan);
        m_polygon.resize(6);
        m_polygon[0].position =
        m_polygon[0].texCoords = {BASE_RADIUS+1, BASE_RADIUS+1};
        m_polygon[1].position =
        m_polygon[5].position =
        m_polygon[1].texCoords =
        m_polygon[5].texCoords = {0.f, 0.f};
        m_polygon[2].position =
        m_polygon[2].texCoords = {BASE_RADIUS*2 + 2, 0.f};
        m_polygon[3].position =
        m_polygon[3].texCoords = {BASE_RADIUS*2 + 2, BASE_RADIUS*2 + 2};
        m_polygon[4].position =
        m_polygon[4].texCoords = {0.f, BASE_RADIUS*2 + 2};
        setRange(1.f);
        setBeamAngle(360.f);
        //Transformable::setOrigin(BASE_RADIUS, BASE_RADIUS);
        m_bounds.width  = BASE_RADIUS * 2.f;
        m_bounds.height = BASE_RADIUS * 2.f;
        instance_count++;
        */
        m_states.custom_draw_fn = SP_LAMBDA_CAPTURE_EQ_THIS(){draw();};
    }

    RadialLight::~RadialLight()
    {
        instance_count--;
    }

    void RadialLight::initializeTextures()
    {
        int points = 100;
        texture_fade.create(BASE_RADIUS * 2 + 2, BASE_RADIUS * 2 + 2);
        texture_plain.create(BASE_RADIUS * 2 + 2, BASE_RADIUS * 2 + 2);

        VertexArray lightShape(TriangleFan, points + 2);
        float step = PI * 2.f / points;
        lightShape[0].position = vec2f{BASE_RADIUS + 1, BASE_RADIUS + 1};

        for(int i = 0; i < points + 2; i++)
        {
            lightShape[i].position =
            {
                (std::sin(step*(i)) + 1) * BASE_RADIUS + 1,
                (std::cos(step*(i)) + 1) * BASE_RADIUS + 1
            };

            lightShape[i].color.a = 0;
        }

        //do transient draw to FBO..
        States states;

        texture_fade.bind();
        Drawable::draw(lightShape.getVertices(),
                       lightShape.length(),
                       BASE_RADIUS * 2 + 2,
                       BASE_RADIUS * 2 + 2);
        texture_fade.display();
        fade_texture = texture_fade.copyColorToTexture();
        fade_texture.setSmooth(true);

        /*
        m_draw_states.texture = &fade_texture;
        m_draw_states.blend_mode = BlendAdd;
        */

    }

    RadialLight::Ptr RadialLight::create()
    {
        return Ptr(new RadialLight);
    }
    void RadialLight::setPosition(float x, float y)
    {
        Drawable::setPosition(x, y);
        m_transformable.setPosition(x, y);
    }
    void RadialLight::resetColor()
    {
        VertexArray::setColor(m_polygon, m_color);
    }

    float module360(float x){
        x = (float)fmod(x,360.f);
        if(x < 0.f) x += 360.f;
        return x;
    }

    void RadialLight::setBeamAngle(float r)
    {
        m_beam_angle = module360(r);
    }

    rectf RadialLight::getGlobalBounds() const
    {
        float scale_range = m_range / BASE_RADIUS;
        m_transformable.setPosition(m_position);
        const_cast<mat&>(m_transformable.getMatrix()).scale(scale_range, scale_range, BASE_RADIUS, BASE_RADIUS);
        return m_transformable.getMatrix().transformRect(Drawable::getGlobalBounds());
    }
    void RadialLight::draw()
    {
        //printf("custom draw..\n");

        const mat& matrix = m_transformable.getMatrix();
        const_cast<mat&>(m_transformable.getMatrix()).scale(m_range/BASE_RADIUS, m_range/BASE_RADIUS, BASE_RADIUS, BASE_RADIUS);
        m_draw_states.matrix  = matrix;
        m_draw_states.texture = &fade_texture;
        m_draw_states.blend_mode = BlendAdd;
        Drawable::draw(m_polygon.getVertices(), m_polygon.length(), BASE_RADIUS * 2 + 2, BASE_RADIUS * 2 + 2, m_draw_states, true);
    }

    void RadialLight::cast(const EdgeVector::iterator& begin, const EdgeVector::iterator& end)
    {
        float scale_range = m_range / BASE_RADIUS;

        m_transformable.setPosition(m_position);
        const_cast<mat&>(m_transformable.getMatrix()).scale(scale_range, scale_range, BASE_RADIUS, BASE_RADIUS);
        std::vector<sp::Line> rays;
        rays.reserve(2 + std::distance(begin, end) * 2 * 3);

        float bl1 = module360(m_transformable.getRotation() - m_beam_angle / 2);
        float bl2 = module360(m_transformable.getRotation() - m_beam_angle / 2);
        vec2f cast_point = m_transformable.getPosition();
        float off = .001f;

        bool beam_angle_big_enough = m_beam_angle < 0.1f;
        auto angle_in_beam = [&](float a)->bool
        {
            return beam_angle_big_enough
                || (bl1 < bl2 && a > bl1 && a < bl2)
                || (bl1 > bl2 && (a > bl1 || a < bl2));
        };

        for(float a = 45.f; a < 360.f; a += 90.f)
        {
            if(beam_angle_big_enough || angle_in_beam(a))
                rays.emplace_back(cast_point, a);
        }

        rectf light_bounds = getGlobalBounds();
        for(auto it = begin; it != end; it++)
        {
            auto& s = *it;
            if(light_bounds.intersects(s.getGlobalBounds()))
            {
                Line r1(cast_point, s.origin);
                Line r2(cast_point, s.point(1.f));

                float a1 = angle(r1.direction);
                float a2 = angle(r2.direction);

                if(angle_in_beam(a1))
                {
                    rays.push_back(r1);
                    rays.emplace_back(cast_point, a1 - off);
                    rays.emplace_back(cast_point, a1 + off);
                }

                if(angle_in_beam(a2))
                {
                    rays.push_back(r2);
                    rays.emplace_back(cast_point, a2 - off);
                    rays.emplace_back(cast_point, a2 + off);
                }
            }
        }

        if(bl1 > bl2)
        {
            std::sort(rays.begin(),
                      rays.end(),
            [bl1, bl2](Line& r1, Line& r2)
            {
                float _bl1 = bl1 - 0.1;
                float _bl2 = bl2 + 0.1;
                float a1 = angle(r1.direction);
                float a2 = angle(r2.direction);
                return (a1 >= _bl1 && a2 <= _bl2) || (a1 < a2 && (_bl1 <= a1 || a2 <= _bl2));
            });
        }
        else
        {
            std::sort(rays.begin(),
                      rays.end(),
            [bl1](Line& r1, Line& r2)
            {
                return angle(r1.direction) < angle(r2.direction);
            });
        }

        mat tr_i = !m_transformable.getMatrix();

        std::vector<vec2f> points;
        points.reserve(rays.size());
        for(auto& r : rays)
        {
            points.push_back(tr_i.transformVertex(cast_ray(begin, end, r, m_range * m_range)));
        }

        m_polygon.resize(points.size() + 1 + beam_angle_big_enough);
        m_polygon[0].color = m_color;
        m_polygon[0].position = m_polygon[0].texCoords = tr_i.transformVertex(cast_point);

        for(unsigned i = 0; i < points.size(); i++)
        {
            vec2f p = points[i];
            m_polygon[i + 1].position = p;
            m_polygon[i + 1].texCoords = p;
            m_polygon[i + 1].color = m_color;
        }

        if(beam_angle_big_enough)
        {
            m_polygon[points.size() + 1] = m_polygon[1];
        }
    }
}
#undef PI
