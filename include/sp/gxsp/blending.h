#ifndef BLENDING_H
#define BLENDING_H

namespace sp
{
    struct Blending
    {
        enum SP_Factor
        {
            Zero,
            One,
            SrcColor,
            OneMinusSrcColor,
            DstColor,
            OneMinusDstColor,
            SrcAlpha,
            OneMinusSrcAlpha,
            DstAlpha,
            OneMinusDstAlpha
        };

        enum SP_Equation
        {
            Add,
            Subtract,
            ReverseSubtract
        };

        Blending();
        Blending(SP_Factor src, SP_Factor dst, SP_Equation = Add);
        Blending(SP_Factor src, SP_Factor dst, SP_Equation,
                 SP_Factor alph_src, SP_Factor alph_dst, SP_Equation);

        SP_Factor      colorSrcFactor;
        SP_Factor      colorDstFactor;
        SP_Equation    colorEquation;
        SP_Factor      alphaSrcFactor;
        SP_Factor      alphaDstFactor;
        SP_Equation    alphaEquation;
    };

    bool operator==(const Blending& b1, const Blending& b2);
    bool operator!=(const Blending& b1, const Blending& b2);

    extern const Blending BlendAlpha;
    extern const Blending BlendAdd;
    extern const Blending BlendMultiply;
    extern const Blending BlendNone;

}

#endif // BLENDING_H
