#include <sp/gxsp/render_states.h>

namespace sp
{
    bool operator==(const States& s1, const States& s2)
    {
        return
        (
            s1.texture          == s2.texture
        &&  s1.shader           == s2.shader
        &&  s1.primitive_type   == s2.primitive_type
        &&  !spHelperCompareFloat(s1.point_size, s2.point_size)
        &&  s1.lighting         == s2.lighting
        &&  s1.viewport         == s2.viewport
        );
    }
}
