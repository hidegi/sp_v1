#ifndef SP_MATH_H
#define SP_MATH_H
#ifndef SP_NO_DEFINE_PRECOMPUTE
#define SPI 				(3.14159265358979323846) ///precomputed constant..
#define SPI2				(SPI * 2.0)
#define SP_PI_DIV_180		(0.017453292519943296)
#define SP_INV_PI_DIV_180	(57.2957795130823229)

#define spDegToRad(r)	((r) * SP_PI_DIV_180)
#define spRadToDeg(r)	((r) * SP_INV_PI_DIV_180)
#endif
#include <cmath>

namespace sp
{
	inline unsigned int spNextP2(unsigned int val)
	{
		unsigned int n = 1;
		while(n < val) n = n << 1;
		return n;
	}

	template<typename T, SP_ENABLE_IF(IS_ARITHMETIC(T))>
	inline T clamp(T val, T low, T high)
	{
		//return (val < low) ? low : (val > high) ? high : val;
		return std::max(low, std::min(val, high));
	}

	inline float fclamp(float val, float low, float high)
	{
	    return std::fmax(low, std::fmin(val, high));
	}
}
#endif
