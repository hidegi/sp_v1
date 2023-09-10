#ifndef PARTICLE_H
#define PARTICLE_H
#include <sp/sp.h>
#include <sp/gxsp/drawable.h>
#include <memory>

namespace sp
{
    class Cluster : public Drawable
    {
        private:
            struct Particle
            {
                vec2f initial_pos   = {0.f, 0.f};
                vec2f position      = {0.f, 0.f};

                float lifetime;
                int seed1;
                int seed2;
                sp::Color color = sp::Color{255, 255, 255, 255};
            };

            const float MAX_TIME = 1e+10;
            float currTime = 0.0f;

            std::vector<Particle>   m_particles;

            vec2f   m_src_position;
            vec2f   m_prev_src_position;
            vec2f   m_next_src_position;
            float   m_avg_lifetime;
            bool    m_interpolant;

        public:
            typedef std::shared_ptr<Cluster>           Ptr;
            typedef std::shared_ptr<const Cluster>     ConstPtr;
            //typedef std::funciton<vec2f(vec2f)>         ParticleFn;

            static Ptr          create(size_t amount, float avg_life_time, const sp::vec2f& pos, const Color& color = {255, 255, 255, 255});
            static Ptr          create(size_t amount, float avg_life_time, const sp::vec2f& pos, const Texture& texture, const Color& color = {255, 255, 255, 255});

            void                setColor(const Color& color);
            //an invokable drawable..
            void                update(double dt, const vec2f& position);

        private:
            friend void evaluate(float t, sp::Cluster::Particle& p);
            void                init();
                                Cluster(size_t amount, float avg_life_time, const sp::vec2f& pos, const Color& color = {255, 255, 255, 255});
                                Cluster(size_t amount, float avg_lifetime, const sp::vec2f& pos, const Texture& texture, const Color& color = {255, 255, 255, 255});

            Color               m_color;
            size_t              m_count;
    };
}

#endif // PARTICLE_H
