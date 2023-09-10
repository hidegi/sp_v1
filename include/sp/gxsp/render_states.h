#ifndef RENDER_STATES_H_INCLUDED
#define RENDER_STATES_H_INCLUDED
#include <sp/sp.h>
#include <sp/gxsp/blending.h>
#include <sp/gxsp/viewport.h>
#include <sp/utils/helpers.h>
#include <functional>

namespace sp
{
    class Texture;
    class Shader;

    struct SP_API States
    {
        const sp::Texture*      texture;
        const sp::Shader*       shader;
        int                     primitive_type;
        float                   point_size;
        Blending                blend_mode;
        bool                    lighting;
        std::function<void()>   custom_draw_fn;
        bool                    custom_draw_enable;

        //Viewport                viewport;
        const Viewport*         viewport;

        States() :
            texture             {nullptr},
            shader              {nullptr},
            primitive_type      {Triangles},
            point_size          {1.f},
            blend_mode          {BlendAlpha},
            lighting            {false},
            custom_draw_fn      {nullptr},
            custom_draw_enable  {false},
            viewport            {nullptr}
        {
        }
        States(const States& other) :
            texture             {other.texture},
            shader              {other.shader},
            primitive_type      {other.primitive_type},
            point_size          {other.point_size},
            blend_mode          {other.blend_mode},
            lighting            {other.lighting},
            custom_draw_fn      {other.custom_draw_fn},
            custom_draw_enable  {other.custom_draw_enable},
            viewport            {other.viewport}
        {
        }

        States& operator=(const States& other)
        {
            if(this != &other)
            {
                texture             = other.texture;
                shader              = other.shader;
                primitive_type      = other.primitive_type;
                point_size          = other.point_size;
                blend_mode          = other.blend_mode;
                lighting            = other.lighting;
                custom_draw_fn      = other.custom_draw_fn;
                custom_draw_enable  = other.custom_draw_enable;
                viewport            = other.viewport;
            }
            return *this;
        }
    };

    bool operator==(const States& s1, const States& s2);
}

#endif // RENDER_STATES_H_INCLUDED
