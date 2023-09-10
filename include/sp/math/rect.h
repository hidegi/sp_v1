#ifndef RECT_H_INCLUDED
#define RECT_H_INCLUDED
#include <sp/sp.h>
#include <sp/math/vec.h>

namespace sp
{
    template <typename T>
    struct rect
    {
    public:
        SP_CONSTEXPR rect() = default;

        template <typename U>
        explicit SP_CONSTEXPR rect(const rect<U>& r) :
            left  {static_cast<T>(r.left)},
            top   {static_cast<T>(r.top)},
            width {static_cast<T>(r.width)},
            height{static_cast<T>(r.height)}
        {
        }

		template <typename U>
		explicit SP_CONSTEXPR rect(U l, U t, U w, U h) :
			left	{static_cast<T>(l)},
			top		{static_cast<T>(t)},
			width	{static_cast<T>(w)},
			height	{static_cast<T>(h)}
		{
		}
        explicit SP_CONSTEXPR rect(T rectLeft, T rectTop, T rectWidth, T rectHeight) :
            left  {rectLeft},
            top   {rectTop},
            width {rectWidth},
            height{rectHeight}
        {
        }

        SP_CONSTEXPR rect(vec2<T> position, vec2<T> size) :
            left  {position.x},
            top   {position.y},
            width {size.x},
            height{size.y}
        {
        }

        SP_CONSTEXPR void setPosition(vec2<T> position)
        {
            left = position.x;
            top = position.y;
        }

        SP_CONSTEXPR vec2<T> getPosition() const
        {
            return {left, top};
        }

        SP_CONSTEXPR void setSize(vec2<T> size)
        {
            width = size.x;
            height = size.y;
        }

        SP_CONSTEXPR vec2<T> getSize() const
        {
            return {width, height};
        }

        SP_CONSTEXPR bool contains(const vec2<T>& pos) const
        {
            return (pos.x >= left) && (pos.x < left + width) && (pos.y >= top) && (pos.y < top + height);
        }

        SP_CONSTEXPR bool contains(T x, T y) const
        {
            return (x >= left) && (x < left + width) && (y >= top) && (y < top + height);
        }

		rect<T>& operator=(const rect<T>& other)
		{
			left = other.left;
			top = other.top;
			width = other.width;
			height = other.height;
			return *this;
		}

        SP_CONSTEXPR bool intersects(const rect<T>& r) const
        {
            // Compute the intersection boundaries
            const T interLeft   = std::max(left, r.left);
            const T interTop    = std::max(top, r.top);
            const T interRight  = std::min(left + width, r.left + r.width);
            const T interBottom = std::min(top + height, r.top + r.height);

            // If the intersection is valid (positive non zero area), then there is an intersection
            return (interLeft < interRight) && (interTop < interBottom);
        }

    public:

        T left = 0;   //!< Left coordinate of the rectangle
        T top = 0;    //!< Top coordinate of the rectangle
        T width = 0;  //!< Width of the rectangle
        T height = 0; //!< Height of the rectangle
    };

    template <typename T>
    SP_CONSTEXPR bool operator==(const rect<T>& left, const rect<T>& right)
    {
        return (left.left == right.left) && (left.width == right.width)
            && (left.top == right.top) && (left.height == right.height);
    }

    template <typename T>
    SP_CONSTEXPR bool operator!=(const rect<T>& left, const rect<T>& right)
    {
        return !(left == right);
    }

    template<>
    inline bool operator==<float>(const rect<float>& r1, const rect<float>& r2)
    {
        return (!spHelperCompareFloat(r1.left, r2.left) &&
                !spHelperCompareFloat(r1.width, r2.width) &&
                !spHelperCompareFloat(r1.top, r2.top) &&
                !spHelperCompareFloat(r1.height, r2.height));
    }

    template<>
    inline bool operator!=<float>(const rect<float>& r1, const rect<float>& r2)
    {
        return !(r1 == r2);
    }

    using rectf = rect<float>;
    using recti = rect<int>;
    using rectu = rect<unsigned int>;
}

#endif // RECT_H_INCLUDED
