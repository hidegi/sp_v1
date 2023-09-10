#ifndef SHADER_H
#define SHADER_H
#include <sp/sp.h>
#include <sp/gxsp/spglsl.h>
#include <sp/string.h>
#include <map>
namespace sp
{
    class SP_API Shader
    {
        public:
            enum class SP_Program
            {
                Vertex,
                Fragment
            };
                            Shader();
                           ~Shader();

                            Shader(const Shader&) = delete;
                            Shader(Shader&&) = delete;

            Shader&         operator=(const Shader&) = delete;
            Shader&         operator=(Shader&&) = delete;
            bool            loadFromFile(const char* filename, SP_Program pr);
            bool            loadFromFile(const char* vfile, const char* ffile);

            bool            loadFromMemory(const void* data, SP_Program pr);
            bool            loadFromMemory(const void* vdata, const void* fdata);

            bool            attachShaderProgram(const void* data, SP_Program pr);

            void            setUniform(const char* name, float val);
            void            setUniform(const char* name, const gl::vec2f& val);
            void            setUniform(const char* name, const gl::vec3f& val);
            void            setUniform(const char* name, const gl::vec4f& val);

            void            setUniform(const char* name, int val);
            void            setUniform(const char* name, const gl::vec2i& val);
            void            setUniform(const char* name, const gl::vec3i& val);
            void            setUniform(const char* name, const gl::vec4i& val);

            void            setUniform(const char* name, bool val);
            void            setUniform(const char* name, const gl::vec2b& val);
            void            setUniform(const char* name, const gl::vec3b& val);
            void            setUniform(const char* name, const gl::vec4b& val);

            void            setUniform(const char* name, const gl::mat3& val);
            void            setUniform(const char* name, const gl::mat4& val);

            void            setUniformTexture(const char* name, unsigned texture);

            void            setUniformArray(const char* name, const float* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec2f* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec3f* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec4f* pointer, size_t length);

            void            setUniformArray(const char* name, const int* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec2i* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec3i* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::vec4i* pointer, size_t length);

            void            setUniformArray(const char* name, const gl::mat3* pointer, size_t length);
            void            setUniformArray(const char* name, const gl::mat4* pointer, size_t length);


            unsigned int    getHandleGL() const;
            unsigned int    getCurrentTexture() const;

            static void     bind(const Shader*);
            static bool     shader_objects_supported();
        private:
            friend class Uniform;
            bool            compile(const char* shader);
            bool            compile(const char* vshader, const char* fshader);
            void            bindTextures() const;
            int             getUniformLocation(const char* name);

            std::map<int, unsigned int>     m_texture_map;
            std::map<String, unsigned int>  m_uniform_map;
            unsigned int                    m_native_shader;
            mutable  int                    m_current_texture;
            SP_Program                      m_type;
    };
}
#endif // SHADER_H
