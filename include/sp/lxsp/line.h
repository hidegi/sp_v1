#ifndef LINE_H
#define LINE_H
#include <limits>
#include <sp/math/vec.h>
#include <sp/math/rect.h>


namespace sp
{
    struct Line
    {
        vec2f   origin;
        vec2f   direction;

        Line(const vec2f& p1, const vec2f& p2);
        Line(const vec2f& p, float angle);

        rectf   getGlobalBounds() const;
        int     relativePosition(const vec2f& point) const;
        float   distance(const vec2f& point) const;
        bool    intersection(const Line& line) const;
        bool    intersection(const Line& line, float& normal) const;
        bool    intersection(const Line& line, float& normal1, float& normal2) const;
        vec2f   point(float param) const;
    };

    template<typename It>
    vec2f cast_ray(const It& begin, const It& end, Line ray,
                   float max_range = std::numeric_limits<float>::infinity())
    {
        float min_range = max_range;
        ray.direction = ray.direction.normalized();
        for(auto it = begin; it != end; it++)
        {
            float t_seg, t_ray;
            if(
                it->intersection(ray, t_seg, t_ray)
            &&  t_ray <= min_range
            &&  t_ray >= 0.f
            &&  t_seg <= 1.f
            &&  t_seg >= 0.f
              )
            {
                min_range = t_ray;
            }
        }

        return ray.point(min_range);
    }
}

#endif // LINE_H
