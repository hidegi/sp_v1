#ifndef SP_COLOR_H
#define SP_COLOR_H
#include <sp/sp.h>
namespace sp
{
	struct SP_API Color
	{
		Color(SPuint8 r = 255, SPuint8 g = 255, SPuint8 b = 255, SPuint8 a = 255) :
			r(r),
			g(g),
			b(b),
			a(a)
		{
		}
		SPuint8 r;
		SPuint8 g;
		SPuint8 b;
		SPuint8 a;
	};

	SP_API SP_CONSTEXPR bool operator==(const Color& c1, const Color& c2)
    {
        return {
               c1.r == c2.r
            && c1.g == c2.g
            && c1.b == c2.b
            && c1.a == c2.a
        };
    }
}
#endif
