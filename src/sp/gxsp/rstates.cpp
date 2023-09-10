#include <sp/gxsp/rstates.h>

namespace sp
{
    const States States::DEFAULT
    {
        Blending
        {
            Blending::SrcAlpha,
            Blending::OneMinusSrcAlpha,
            Blending::Add,
            Blending::One,
            Blending::OneMinusSrcAlpha,
            Blending::Add
        }
    };

    States::States() :
        blending    {BlendAlpha},
        matrix      {},
        texture     {nullptr},
        shader      {nullptr}
    {
    }



    States::States(Blending b) :
        blending    {b},
        matrix      {},
        texture     {nullptr},
        shader      {nullptr}
    {
    }

    States::States(const mat& m) :
        blending    {BlendAlpha},
        matrix      {m},
        texture     {nullptr},
        shader      {nullptr}
    {
    }

    States::States(const Texture* t) :
        blending    {BlendAlpha},
        matrix      {},
        texture     {t},
        shader      {nullptr}
    {
    }

    States::States(const Shader* s) :
        blending    {BlendAlpha},
        matrix      {},
        texture     {nullptr},
        shader      {s}
    {
    }


    States::States(Blending b, const mat& m, const Texture* t, const Shader* s) :
        blending    {b},
        matrix      {m},
        texture     {t},
        shader      {s}
    {
    }

}
