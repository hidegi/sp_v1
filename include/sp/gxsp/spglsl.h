#ifndef SP_GLSL_H
#define SP_GLSL_H
#include <sp/sp.h>
#include <sp/math/vec.h>
#include <sp/math/mat.h>
#include <sp/gxsp/color.h>
#include <sstream>
#include <cstring>
namespace sp
{
    namespace gl
    {
        template<size_t c, size_t r>
        struct _mat;

        template<typename T>
        struct vec3;

        template<typename T>
        struct vec4;

#include <sp/gxsp/spglsl.inl>

        using vec2f = sp::vec2f;
        using vec2d = sp::vec2d;
        using vec2i = sp::vec2i;
        using vec2u = sp::vec2u;
        using vec2b = sp::vec2b;

        using vec3f = vec3<float>;
        using vec3d = vec3<double>;
        using vec3i = vec3<int>;
        using vec3u = vec3<unsigned int>;
        using vec3b = vec3<bool>;

        using vec4f = vec4<float>;
        using vec4d = vec4<double>;
        using vec4i = vec4<int>;
        using vec4u = vec4<unsigned int>;
        using vec4b = vec4<bool>;

        using mat3  = _mat<3, 3>;
        using mat4  = _mat<4, 4>;

        static std::string formatGLSL(const char* glsl)
        {
            std::stringstream ss;
#ifndef SP_GLSL_USE_SAFE
            char* input = new char[strlen(glsl) + 1];
            char delim[] = ";";
            strcpy(input, glsl);
            char* line = strtok(input, delim);

            while(line)
            {
                ss << line << ";\n";
                line = strtok(0, delim);
            }
            delete [] input;
#else
            size_t len = strlen(glsl) + 1;
            char* input = new char[len];
            char delim[] = ";";
            memcpy(input, glsl, len);
            input[len - 1] = 0;
            rsize_t strmax = sizeof(input);
            char* next_token{};
            char* line = strtok_s(input, delim, &next_token);
            while(line)
            {
                ss << line << ";\n";
                line = strtok_s(0, delim, &next_token);
            }
#endif
            std::string output = ss.str();
            size_t found = output.find('\n');
            output.erase(found + 1, 1);
            output.erase(output.length() - 3, 2);
            return output;
        }
    }
}

#define SP_EMBED_TO_STR(...) #__VA_ARGS__
#define SP_GLSL(v, ...) sp::gl::formatGLSL("#version " #v "\n" SP_EMBED_TO_STR(#__VA_ARGS__))

#endif // SP_GLSL_H
