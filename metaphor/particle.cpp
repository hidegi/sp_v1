#include <sp/gxsp/particle.h>

namespace sp
{
    void evaluate(float t, sp::Cluster::Particle& p)
    {
        float constant = 1.0f * (float)(p.seed1 - RAND_MAX/2)/(float)RAND_MAX;
        float constant2 = 1.0f * (float)(p.seed2 - RAND_MAX/2)/(float)RAND_MAX;

        float f1 = 20.0f*log(1.0f + 1.0f*t);
        float f2 = 20.0f*log(1.0f + 1.0f*t);

        float random = ((rand() % 100) - 50) / 10.f;
        float x = random; //1.0f*constant * f1;
        float y = random;//1.0f*constant2 * f2;


        uint8_t value = 255/(1.0f + 0.1f*t);
        p.color = sp::Color(value, value, value);
        p.position = sp::vec2f{x,y} + p.initial_pos;
    };


    Cluster::Ptr Cluster::create(size_t amount, float avg_life_time, const sp::vec2f& pos, const Color& color)
    {
        return Ptr(new Cluster(amount, avg_life_time, pos, color));
    }
    Cluster::Ptr Cluster::create(size_t amount, float avg_life_time, const sp::vec2f& pos, const Texture& texture, const Color& color)
    {
        return Ptr(new Cluster(amount, avg_life_time, pos, texture, color));
    }

    Cluster::Cluster(size_t amount, float avg_life_time, const sp::vec2f& pos, const Color& color) :
        Drawable(),
        m_count{amount},
        m_color {color},
        m_avg_lifetime{avg_life_time},
        m_interpolant{false},
        m_src_position   {pos}
    {
        m_prev_src_position = m_src_position;
        m_next_src_position = m_src_position;

        m_primitive_type = Points;
        m_vertices.reserve(amount);
        m_indices.reserve(amount);
        init();
    }

    Cluster::Cluster(size_t amount, float avg_life_time, const sp::vec2f& pos, const Texture& texture, const Color& color) :
        Drawable(),
        m_count {amount},
        m_color {color},
        m_avg_lifetime{avg_life_time},
        m_interpolant{false},
        m_src_position{pos}
    {
        m_prev_src_position = m_src_position;
        m_next_src_position = m_src_position;

        m_primitive_type = Points;
        texture.generateMipmap();
        setTexture(texture);
        m_vertices.reserve(amount);
        m_indices.reserve(amount);

        init();
    }

    void Cluster::setColor(const Color& color)
    {
        m_color = color;
        m_update = true;
    }

    void Cluster::init()
    {
        for(size_t i = 0; i < m_count; i++)
        {
            Particle p;
            p.seed1 = rand();
            p.seed2 = rand();
            p.lifetime = m_avg_lifetime * rand() / RAND_MAX;
            p.position = m_src_position;
            p.initial_pos = m_src_position;
            p.color = m_color;

            m_particles.push_back(p);
            m_vertices.push_back
            (
                sp::Vertex(m_src_position, m_color, vec2f{0.f, 0.f})
            );
            m_indices.push_back(i);
        }
    }

    void Cluster::update(double dt, const vec2f& position)
    {
        if(m_interpolant)
        {
            m_prev_src_position = m_next_src_position;
            m_next_src_position = position;
            m_src_position      = m_prev_src_position;
        }
        else
        {
            m_src_position = position;
        }


        for(size_t i = 0; i < m_count; i++)
        {
            /*
            SPuint8 alpha;
            m_vertices[i].position = defaultPos(dt, position, alpha);
            m_vertices[i].color.a = alpha;
            */

            float newTime = fmodf(currTime + dt, m_particles[i].lifetime);
            if(newTime < fmodf(currTime, m_particles[i].lifetime))
            {
                m_particles[i].seed1 = rand();
                m_particles[i].seed2 = rand();
                if(m_interpolant)
                {
                    float proportion = (float) i / (float)(m_particles.size() - 1);
                    m_particles[i].initial_pos = proportion* (m_next_src_position - m_prev_src_position) + m_prev_src_position;
                }
                else
                    m_particles[i].initial_pos = m_src_position;
            }

            evaluate(newTime, m_particles[i]);
            m_vertices[i].position = m_particles[i].position;
            m_vertices[i].color    = m_particles[i].color;
            m_vertices[i].color.a  -= (unsigned int) std::floor(dt* 2.5f * 255 + .5f);

            if((i == m_particles.size() - 1) && m_interpolant)
                m_prev_src_position = m_next_src_position;
        }

        currTime += dt;
        if(currTime > MAX_TIME)
            currTime = 0.0f;
        m_update = true;
    }
}
