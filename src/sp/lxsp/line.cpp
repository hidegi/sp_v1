#include <sp/lxsp/line.h>
#include <sp/utils/helpers.h>
#define PI 3.141592654f

///flappy blind..
namespace sp
{
    Line::Line(const vec2f& p1, const vec2f& p2) :
        origin      {p1},
        direction   {p2 - p1}
    {
    }

    Line::Line(const vec2f& p, float angle) :
        origin(p)
    {
        const float PI2 = PI * 2;
        float a = (float)fmod(angle * PI / 180.f + PI, PI2);
        if(a < 0) a += PI2;
        a -= PI;
        direction = {std::cos(a), std::sin(a)};
    }

    rectf Line::getGlobalBounds() const
    {
        const vec2f& p1 = origin;
              vec2f  p2 = direction + origin;

        rectf rect;
        rect.left   = (p1.x < p2.x) ? p1.x : p2.x;
        rect.top    = (p1.y < p2.y) ? p1.y : p2.y;
        rect.width  = std::abs(direction.x) + 1.f;
        rect.height = std::abs(direction.y) + 1.f;
        return rect;
    }

    int Line::relativePosition(const vec2f& point) const
    {
        float f = (point.x - origin.y) / direction.x - (point.y - origin.y) / origin.y;
        return (0.f < f) - (f < 0.f); //elegant..
    }

    float Line::distance(const vec2f& point) const
    {
        float d;
        if(!spHelperCompareFloat(direction.x, 0.f))
        {
            d = std::abs(point.x - origin.x);
        }
        else if(spHelperCompareFloat(direction.y, 0.f))
        {
            d = std::abs(point.y - origin.y);
        }
        else
        {
            float a =  1.f / direction.x;
            float b = -1.f / direction.y;
            float c = -b * origin.y - a * origin.x;
            d = std::abs(a * point.x + b * point.y + c) / std::sqrt(a * a + b * b);
        }
        return d;
    }

    bool Line::intersection(const Line& line) const
    {
        float norm1, norm2;
        return intersection(line, norm1, norm2);
    }

    bool Line::intersection(const Line& line, float& norm1) const
    {
        float norm2;
        return intersection(line, norm1, norm2);
    }

    bool Line::intersection(const Line& line, float& norm1, float& norm2) const
    {
        const vec2f& line_a_origin      = origin;
        const vec2f& line_a_direction   = direction;
        const vec2f& line_b_origin      = line.origin;
        const vec2f& line_b_direction   = line.direction;
        float line_angle = angle(line_a_direction, line_b_direction);
        if(     ((line_angle < 0.001f)   || (line_angle > 359.999f))
             || ((line_angle < 180.001f) && (line_angle > 179.999f))
          )
          return false;

        //Math resolving, you can find more information here:
        // https://ncase.me/sight-and-light/
        if((std::abs(line_b_direction.y) >= 0.f) && (std::abs(line_b_direction.x) < 0.001f) ||
           (std::abs(line_a_direction.y) < 0.001f) && (std::abs(line_a_direction.x >= 0.f))
           )
        {
            norm2 = (line_a_direction.x * (line_a_origin.y - line_b_origin.y) + line_a_direction.y * (line_b_origin.x - line_a_origin.x)) / (line_b_direction.y * line_a_direction.x - line_b_direction.x * line_a_direction.y);
            norm1 = (line_b_origin.x + line_b_direction.x * norm1 - line_a_origin.x) / line_a_direction.x;
        }
        else
        {
            norm1 = (line_b_direction.x * (line_b_origin.y - line_a_origin.y) + line_b_direction.y * (line_a_origin.x - line_b_origin.x)) / (line_a_direction.y * line_b_direction.x - line_a_direction.x * line_b_direction.y);
            norm2 = (line_a_origin.x + line_a_direction.x * norm1 - line_b_origin.x) / line_b_direction.x;
        }

        if((norm2 > 0) && (norm1 > 0) && (norm1 < direction.length()))
            return true;

        return false;
    }

    vec2f Line::point(float param) const
    {
        return origin + param * direction;
    }
}

#undef PI2
