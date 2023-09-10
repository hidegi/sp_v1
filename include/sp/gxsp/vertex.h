#ifndef VERTEX_H_INCLUDED
#define VERTEX_H_INCLUDED

#include <sp/math/vec.h>
#include <sp/gxsp/color.h>
#include <sp/utils/helpers.h>
namespace sp
{
	struct SP_API Vertex
	{
		Vertex(const sp::vec2f& pos = {0.f, 0.f}, const Color& c = {255, 255, 255}, const sp::vec2f tc = {0.f, 0.f}) :
			position(pos),
			color(c),
			texCoords(tc)
		{
		}

		vec2f	position {0, 0};
		Color	color	 {255, 255, 255};
		vec2f	texCoords{0, 0};

		bool operator==(const Vertex& v2)
        {
            return
            (
              !spHelperCompareFloat(position.x, v2.position.x)
        &&    !spHelperCompareFloat(position.y, v2.position.y)
        /*
        &&    !spHelperCompareFloat(texCoords.x, v2.texCoords.x)
        &&    !spHelperCompareFloat(texCoords.y, v2.texCoords.y)
        &&    (color.r == v2.color.r)
        &&    (color.g == v2.color.g)
        &&    (color.b == v2.color.b)
        &&    (color.a == v2.color.a)
        */
             );
        }
	};


}


#endif // VERTEX_H_INCLUDED
