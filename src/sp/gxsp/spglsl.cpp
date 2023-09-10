#include <sp/gxsp/spglsl.h>

namespace sp
{
///===============================================================================
/// stolen code.. :>
///===============================================================================

    namespace gl {
    void spCopyMat(const mat& src, _mat<3, 3>& dst)
    {
        const float* from = src();
        float* to = dst.pointer;
        to[0] = from[ 0]; to[1] = from[ 1]; to[2] = from[ 3];
        to[3] = from[ 4]; to[4] = from[ 5]; to[5] = from[ 7];
        to[6] = from[12]; to[7] = from[13]; to[8] = from[15];
    }

    void spCopyMat(const mat& src, _mat<4, 4>& dst)
    {
        spCopyMat(src(), 4 * 4, dst.pointer);
    }

    void spCopyMat(const float* src, size_t length, float* dst)
    {
        std::copy(src, src + length, dst);
    }

    void spCopyVec3(const sp::Color& src, vec3<float>& dst)
    {
        dst.x = src.r / 255.f;
        dst.y = src.g / 255.f;
        dst.z = src.b / 255.f;
    }

    void spCopyVec3(const sp::Color& src, vec3<int>& dst)
    {
        dst.x = static_cast<int>(src.r);
        dst.y = static_cast<int>(src.g);
        dst.z = static_cast<int>(src.b);
    }

    void spCopyVec4(const sp::Color& src, vec4<float>& dst)
    {
        dst.x = src.r / 255.f;
        dst.y = src.g / 255.f;
        dst.z = src.b / 255.f;
        dst.w = src.a / 255.f;
    }

    void spCopyVec4(const sp::Color& src, vec4<int>& dst)
    {
        dst.x = static_cast<int>(src.r);
        dst.y = static_cast<int>(src.g);
        dst.z = static_cast<int>(src.b);
        dst.w = static_cast<int>(src.a);
    }
    }
}
