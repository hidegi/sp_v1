#include <sp/gxsp/blending.h>

namespace sp
{
    const Blending BlendAlpha
    {
        Blending::SrcAlpha,
        Blending::OneMinusSrcAlpha,
        Blending::Add,
        Blending::One,
        Blending::OneMinusSrcAlpha,
        Blending::Add
    };

    const Blending BlendAdd
    {
        Blending::SrcAlpha,
        Blending::One,
        Blending::Add,
        Blending::One,
        Blending::One,
        Blending::Add,
    };

    const Blending BlendMultiply
    {
        Blending::DstColor,
        Blending::Zero
    };

    const Blending BlendNone
    {
        Blending::One,
        Blending::Zero
    };
    Blending::Blending() :
        colorSrcFactor  {Blending::SrcAlpha},
        colorDstFactor  {Blending::OneMinusSrcAlpha},
        colorEquation   {Blending::Add},
        alphaSrcFactor  {Blending::One},
        alphaDstFactor  {Blending::OneMinusSrcAlpha},
        alphaEquation   {Blending::Add}
    {
    }

    Blending::Blending(SP_Factor src, SP_Factor dst, SP_Equation eq) :
        colorSrcFactor  {src},
        colorDstFactor  {dst},
        colorEquation   {eq},
        alphaSrcFactor  {src},
        alphaDstFactor  {dst},
        alphaEquation   {eq}
    {
    }

    Blending::Blending(SP_Factor src, SP_Factor dst, SP_Equation eq,
                       SP_Factor a_src, SP_Factor a_dst, SP_Equation a_eq) :
        colorSrcFactor  {src},
        colorDstFactor  {dst},
        colorEquation   {eq},
        alphaSrcFactor  {a_src},
        alphaDstFactor  {a_dst},
        alphaEquation   {a_eq}
    {
    }

    bool operator==(const Blending& b1, const Blending& b2)
    {
        return
        (
            (b1.colorSrcFactor == b2.colorSrcFactor)
        &&  (b1.colorDstFactor == b2.colorDstFactor)
        &&  (b1.colorEquation  == b2.colorEquation)
        &&  (b1.alphaSrcFactor == b2.alphaSrcFactor)
        &&  (b1.alphaDstFactor == b2.alphaDstFactor)
        &&  (b1.alphaEquation  == b2.alphaEquation)
        );
    }
    bool operator!=(const Blending& b1, const Blending& b2)
    {
        return !(b1 == b2);
    }
}
