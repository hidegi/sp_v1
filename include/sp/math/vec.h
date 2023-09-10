#ifndef SP_VEC_H
#define SP_VEC_H
#include <sp/sp.h>
#include <sp/utils/helpers.h>
#include <cmath>
namespace sp
{
#ifndef SP_NO_DEFINE_VEC2
#ifndef SP_DEFINE_NO_HELP_TRAITS
	template<typename T, SP_ENABLE_IF_ARITHMETIC(T)>
#else
	template<typename T>
#endif
	struct vec2
	{
		SP_CONSTEXPR vec2() :
			x{0},
			y{0}
		{
		}

		SP_CONSTEXPR vec2(T _x, T _y) :
			x{_x},
			y{_y}
		{
		}

		template<typename U>
		SP_CONSTEXPR vec2(U _x, U _y) :
			x{static_cast<T>(_x)},
			y{static_cast<T>(_y)}
		{
		}

		float length() const
		{
            return std::sqrt(x * x + y * y);
		}

		vec2 normalized() const
		{
		    float len = length();
            return vec2{x / len, y / len};
		}

		template<typename U>
		SP_CONSTEXPR vec2(const vec2<U>& v) :
			x{static_cast<T>(v.x)},
			y{static_cast<T>(v.y)}
		{
		}

		SP_CONSTEXPR vec2(const vec2<T>& v) :
			x{v.x},
			y{v.y}
		{
		}

		SP_CONSTEXPR vec2<T>& operator=(const vec2<T>& v)
		{
			x = v.x;
			y = v.y;
			return *this;
		}

		T x;
		T y;
	};

	template<typename T>
	float dot(const vec2<T>& v1, const vec2<T>& v2)
	{
	    return {v1.x * v2.x + v1.y * v2.y};
	}

	template<typename T>
	float angle(const vec2<T>& v1, const vec2<T>& v2)
    {
        return std::acos(dot(v1, v2) / v1.length() * v2.length()) * 180.f / 3.141592654f;
    }

    template <typename T, typename U>
    float angle(const vec2<T>& v1, const vec2<U>& v2)
    {
        return std::acos(dot(v1, v2) / v1.length() * v2.length()) * 180.f / 3.141592654f;
    }

    template <typename T>
    float angle(const vec2<T>& v)
    {
        return fmod(std::atan2(v.y, v.x) * 180.f / (3.141592654f) + 360.f, 360.f);
    }

    template<typename T>
    float distance(const vec2<T>& v1, const vec2<T>& v2)
    {
        return std::sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
    }
#ifndef SP_VEC2_NO_DEFINE_OPERATORS
	template<typename T>
	SP_CONSTEXPR vec2<T> operator-(const vec2<T>& v);

	template<typename T>
	SP_CONSTEXPR vec2<T> operator+(const vec2<T>& v1, const vec2<T> v2);

	template<typename T>
	SP_CONSTEXPR vec2<T> operator-(const vec2<T>& v1, const vec2<T> v2);

	template<typename T, typename U>
	SP_CONSTEXPR vec2<T> operator+(const vec2<T>& v1, const vec2<U> v2);

	template<typename U, typename T>
	SP_CONSTEXPR vec2<T> operator+(const vec2<U>& v1, const vec2<T> v2);

	template<typename T, typename U>
	SP_CONSTEXPR vec2<T> operator-(const vec2<T>& v1, const vec2<U> v2);

	template<typename U, typename T>
	SP_CONSTEXPR vec2<T> operator-(const vec2<U>& v1, const vec2<U> v2);

	template<typename T>
	SP_CONSTEXPR vec2<T> operator*(const vec2<T>& v, float s);

	template<typename T>
	SP_CONSTEXPR vec2<T> operator*(float s, const vec2<T>& v);

	template<typename T>
	float operator*(const vec2<T>& v1, const vec2<T>& v2);

	template<typename T, typename U>
	float operator*(const vec2<T>& v1, const vec2<U>& v2);

	template<typename T>
	bool operator==(const vec2<T>& v1, const vec2<T>& v2);

	template<typename T, typename U>
	bool operator==(const vec2<T>& v1, const vec2<U>& v2);

	template<typename T>
	bool operator!=(const vec2<T>& v1, const vec2<T>& v2);

	template<typename T, typename U>
	bool operator!=(const vec2<T>& v1, const vec2<U>& v2);

	template<typename T, typename U>
	bool operator!=(const vec2<T>& v1, const vec2<U>& v2);

#endif //endif no define operators..

#ifndef SP_NO_DEFINE_REF_OPERATORS
	template<typename T>
	SP_CONSTEXPR vec2<T>& operator+=(vec2<T>& v1, const vec2<T>& v2);

	template<typename T>
	SP_CONSTEXPR vec2<T>& operator-=(vec2<T>& v1, const vec2<T>& v2);

	template<typename T, typename U>
	SP_CONSTEXPR vec2<T>& operator+=(vec2<T>& v1, const vec2<U>& v2);

	template<typename T, typename U>
	SP_CONSTEXPR vec2<T>& operator-=(vec2<T>& v1, const vec2<U>& v2);

	template<typename T>
	SP_CONSTEXPR vec2<T>& operator*=(vec2<T>& v, float s);

	template<typename T>
	SP_CONSTEXPR vec2<T>& operator*=(float s, vec2<T>& v);

#endif
using vec2f = vec2<float>;
using vec2d = vec2<double>;
using vec2i = vec2<int>;
using vec2u = vec2<unsigned int>;
using vec2b = vec2<bool>;

#endif //endif no define vec2..

#include <sp/math/vec_impl>
}

#endif
