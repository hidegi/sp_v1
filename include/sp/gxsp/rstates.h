#ifndef STATES_H
#define STATES_H
#include <sp/math/mat.h>
#include <sp/gxsp/shader.h>
#include <sp/gxsp/texture.h>
#include <sp/gxsp/blending.h>
namespace sp
{
    class States
    {
        public:
            States();
            States(Blending b);
            States(const mat& m);
            States(const Texture* t);
            States(const Shader* s);
            States(Blending b, const mat& m, const Texture* t, const Shader* s);

            static const States DEFAULT;
            Blending            blending;
            mat                 matrix;
            const Texture*      texture;
            const Shader*       shader;
    };

}

#endif // STATES_H
