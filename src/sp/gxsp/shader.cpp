#include <sp/gxsp/shader.h>
#include <sp/utils/helpers.h>
#include <sp/sp_controller.h>
#include <sp/spgl.h>

#if defined(SP_SYSTEM_MACOS)
    #define to_GLhandle(x) reinterpret_cast<void*>(static_cast<ptrdiff_t>(x))
    #define from_GLhandle(x) static_cast<unsigned int>(reinterpret_cast<ptrdiff_t>(x))
#else
    #define from_GLhandle(x) (x)
    #define to_GLhandle(x) (x)
#endif // defined

namespace sp
{
///===============================================================================
/// stolen code.. :>
///===============================================================================

    namespace
    {
        GLint max_texture_units()
        {
            static GLint max_units = 0;
            static char checked = 0;
            if(!checked)
            {
                checked = true;
                spCheck(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &max_units));
            }
            return max_units;
        }

        template<typename T>
        std::vector<T> flatten(const vec2<T>* vec, size_t length)
        {
            constexpr size_t dim = 2;
            std::vector<T> con(dim * length);

            for(size_t i = 0; i < length; ++i)
            {
                con[dim * i + 0] = vec[i].x;
                con[dim * i + 1] = vec[i].y;
            }

            return con;
        }

        template<typename T>
        std::vector<T> flatten(const gl::vec3<T>* vec, size_t length)
        {
            constexpr size_t dim = 3;
            std::vector<T> con(dim * length);

            for(size_t i = 0; i < length; ++i)
            {
                con[dim * i + 0] = vec[i].x;
                con[dim * i + 1] = vec[i].y;
                con[dim * i + 2] = vec[i].z;
            }

            return con;
        }

        template<typename T>
        std::vector<T> flatten(const gl::vec4<T>* vec, size_t length)
        {
            constexpr size_t dim = 4;
            std::vector<T> con(dim * length);

            for(size_t i = 0; i < length; ++i)
            {
                con[dim * i + 0] = vec[i].x;
                con[dim * i + 1] = vec[i].y;
                con[dim * i + 2] = vec[i].z;
                con[dim * i + 3] = vec[i].w;
            }

            return con;
        }
    }

    struct Uniform
    {
        public:
            Uniform(Shader& shader, const char* name) :
                save_program    {0},
                target_program  {from_GLhandle(shader.m_native_shader)},
                location        {-1}
            {
                if(target_program)
                {
                    spCheck(save_program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB))
                    if(target_program != save_program)
                        spCheck(glUseProgramObjectARB(target_program))

                    location = shader.getUniformLocation(name);
                }
            }

           ~Uniform()
            {
                if(target_program && (target_program != save_program))
                    spCheck(glUseProgramObjectARB(save_program));
            }
            Uniform(const Uniform&) = delete;
            Uniform& operator=(const Uniform&) = delete;
            Uniform(Uniform&&) = delete;
            Uniform& operator=(Uniform&&) = delete;

            GLhandleARB         save_program;
            GLhandleARB         target_program;
            GLint               location;
    };

    Shader::Shader() :
        m_native_shader{0},
        m_current_texture{-1}
    {

    }

    Shader::~Shader()
    {
        if(m_native_shader)
            spCheck(glDeleteObjectARB(m_native_shader))
    }

    bool Shader::loadFromFile(const char* filename, SP_Program pr)
    {
        char* data = reinterpret_cast<char*>(spHelperFileData(filename, NULL));
        if(!data)
        {
            SP_PRINT_WARNING("cannot open file " << filename);
            return false;
        }
        m_type = pr;

        bool status = compile(reinterpret_cast<const char*>(data));
        free(data);
        return status;
    }

    bool Shader::loadFromFile(const char* vfile, const char* ffile)
    {
        char* vdata = reinterpret_cast<char*>(spHelperFileData(vfile, NULL));
        if(!vdata)
        {
            SP_PRINT_WARNING("cannot open vertex file " << vfile);
            return false;
        }

        char* fdata = reinterpret_cast<char*>(spHelperFileData(vfile, NULL));
        if(!fdata)
        {
            SP_PRINT_WARNING("cannot open vertex file " << ffile);
            free(vdata);
            return false;
        }

        bool status = compile(vdata, fdata);
        free(vdata);
        free(fdata);
        return status;
    }

    bool Shader::loadFromMemory(const void* data, SP_Program pr)
    {
        m_type = pr;
        return compile(reinterpret_cast<const char*>(data));
    }

    bool Shader::loadFromMemory(const void* vdata, const void* fdata)
    {
        return compile(reinterpret_cast<const char*>(vdata), reinterpret_cast<const char*>(fdata));
    }
    bool Shader::shader_objects_supported()
    {
        static char checked   = 0;
        static char available = 0;
        if(!checked)
        {
            checked = 1;
            available =
                        GL_ARB_multitexture_supported
                &&      GL_ARB_shader_objects_supported
                &&      GL_ARB_vertex_shader_supported;
        }

        return available;
    }

    bool Shader::compile(const char* source)
    {
        if(!shader_objects_supported())
        {
            SP_PRINT_WARNING("shader objects not supported");
            return false;
        }

        if(m_native_shader)
        {
            spCheck(glDeleteObjectARB(to_GLhandle(m_native_shader)))
            m_native_shader = 0;
        }

        m_current_texture = -1;
        m_texture_map.clear();
        m_uniform_map.clear();

        GLhandleARB shaderProgram;
        spCheck(shaderProgram = glCreateProgramObjectARB());

        GLhandleARB shader;
        spCheck(shader = glCreateShaderObjectARB(m_type == SP_Program::Vertex ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB))
        spCheck(glShaderSourceARB(shader, 1, &source, NULL))
        spCheck(glCompileShaderARB(shader))

        GLint success;
        spCheck(glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &success))
        if(!success)
        {
            char log[1024];
            spCheck(glGetInfoLogARB(shader, sizeof(log), 0, log))
            SP_PRINT_WARNING("shader compilation failed:\n" << log);
            spCheck(glDeleteObjectARB(shader))
            spCheck(glDeleteObjectARB(shaderProgram))
            return false;
        }
        spCheck(glAttachObjectARB(shaderProgram, shader))
        spCheck(glDeleteObjectARB(shader))

        spCheck(glLinkProgramARB(shaderProgram))

        success = 0;
        spCheck(glGetObjectParameterivARB(shaderProgram, GL_OBJECT_LINK_STATUS_ARB, &success))
        if(success == GL_FALSE)
        {
            char log[1024];
            spCheck(glGetInfoLogARB(shaderProgram, sizeof(log), 0, log))
            SP_PRINT_WARNING("shader compilation failed:\n" << log);
            spCheck(glDeleteObjectARB(shaderProgram))
            return false;
        }

        m_native_shader = from_GLhandle(shaderProgram);

        spCheck(glFlush())
        return true;
    }

    bool Shader::compile(const char* vshader, const char* fshader)
    {
        if(!shader_objects_supported())
        {
            SP_PRINT_WARNING("shader objects not supported");
            return false;
        }

        if(m_native_shader)
        {
            spCheck(glDeleteObjectARB(to_GLhandle(m_native_shader)))
            m_native_shader = 0;
        }

        m_current_texture = -1;
        m_texture_map.clear();
        m_uniform_map.clear();

        GLhandleARB shaderProgram;
        spCheck(shaderProgram = glCreateProgramObjectARB());

        if(vshader)
        {
            GLhandleARB shader;
            spCheck(shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB))
            spCheck(glShaderSourceARB(shader, 1, &vshader, NULL))
            spCheck(glCompileShaderARB(shader))

            GLint success;
            spCheck(glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &success))
            if(!success)
            {
                char log[1024];
                spCheck(glGetInfoLogARB(shader, sizeof(log), 0, log))
                SP_PRINT_WARNING("vertex shader compilation failed:\n" << log);
                spCheck(glDeleteObjectARB(shader))
                spCheck(glDeleteObjectARB(shaderProgram))
                return false;
            }
            spCheck(glAttachObjectARB(shaderProgram, shader))
            spCheck(glDeleteObjectARB(shader))
        }

        if(fshader)
        {
            GLhandleARB shader;
            spCheck(shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB))
            spCheck(glShaderSourceARB(shader, 1, &fshader, NULL))
            spCheck(glCompileShaderARB(shader))

            GLint success;
            spCheck(glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &success))
            if(!success)
            {
                char log[1024];
                spCheck(glGetInfoLogARB(shader, sizeof(log), 0, log))
                SP_PRINT_WARNING("fragment shader compilation failed:\n" << log);
                spCheck(glDeleteObjectARB(shader))
                spCheck(glDeleteObjectARB(shaderProgram))
                return false;
            }
            spCheck(glAttachObjectARB(shaderProgram, shader))
            spCheck(glDeleteObjectARB(shader))
        }

        spCheck(glLinkProgramARB(shaderProgram))

        GLint success = 0;
        spCheck(glGetObjectParameterivARB(shaderProgram, GL_OBJECT_LINK_STATUS_ARB, &success))
        if(success == GL_FALSE)
        {
            char log[1024];
            spCheck(glGetInfoLogARB(shaderProgram, sizeof(log), 0, log))
            SP_PRINT_WARNING("shader compilation failed:\n" << log);
            spCheck(glDeleteObjectARB(shaderProgram))
            return false;
        }

        m_native_shader = from_GLhandle(shaderProgram);

        spCheck(glFlush())
        return true;
    }

    int  Shader::getUniformLocation(const char* name)
    {
        auto it = m_uniform_map.find(String(name));
        if(it != m_uniform_map.end())
            return it->second;

        int location = glGetUniformLocationARB(to_GLhandle(m_native_shader), name);
        m_uniform_map.insert(std::make_pair(String(name), location));
        if(location == -1)
            SP_PRINT_WARNING("uniform location " << name << " not found");

        return location;
    }
    void Shader::setUniform(const char* name, float val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform1fARB(binder.location, val));
    }

    void Shader::setUniform(const char* name, const gl::vec2f& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform2fARB(binder.location, val.x, val.y));
    }

    void Shader::setUniform(const char* name, const gl::vec3f& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform3fARB(binder.location, val.x, val.y, val.z));
    }

    void Shader::setUniform(const char* name, const gl::vec4f& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform4fARB(binder.location, val.x, val.y, val.z, val.w));
    }


    void Shader::setUniform(const char* name, int val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform1iARB(binder.location, val));
    }

    void Shader::setUniform(const char* name, const gl::vec2i& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform2iARB(binder.location, val.x, val.y));
    }

    void Shader::setUniform(const char* name, const gl::vec3i& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform3iARB(binder.location, val.x, val.y, val.z));
    }

    void Shader::setUniform(const char* name, const gl::vec4i& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform4iARB(binder.location, val.x, val.y, val.z, val.w));
    }

    void Shader::setUniform(const char* name, bool val)
    {
        setUniform(name, static_cast<int>(val));
    }

    void Shader::setUniform(const char* name, const gl::vec2b& val)
    {
       setUniform(name, static_cast<gl::vec2i>(val));
    }

    void Shader::setUniform(const char* name, const gl::vec3b& val)
    {
        setUniform(name, static_cast<gl::vec3i>(val));
    }

    void Shader::setUniform(const char* name, const gl::vec4b& val)
    {
        setUniform(name, static_cast<gl::vec4i>(val));
    }


    void Shader::setUniform(const char* name, const gl::mat3& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniformMatrix3fvARB(binder.location, 1, GL_FALSE, val.pointer));
    }

    void Shader::setUniform(const char* name, const gl::mat4& val)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniformMatrix4fvARB(binder.location, 1, GL_FALSE, val.pointer));
    }



    void Shader::setUniformTexture(const char* name, unsigned texture)
    {
        if(m_native_shader)
        {
            int location = getUniformLocation(name);
            if(location != -1)
            {
                auto it = m_texture_map.find(location);
                if(it == m_texture_map.end())
                {
                    int max_units = max_texture_units();
                    if(m_texture_map.size() + 1 >= static_cast<size_t>(max_units))
                    {
                        SP_PRINT_WARNING("cannot use texture " << texture << ", since exceeding max texture units (" << max_units << ")");
                        return;
                    }
                    m_texture_map[location] = texture;
                }
                else
                {
                    it->second = texture;
                }
                //m_current_texture = location;
            }
        }
    }

    void Shader::setUniformArray(const char* name, const float* pointer, size_t length)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform1fvARB(binder.location, static_cast<GLsizei>(length), pointer))
    }


    void Shader::setUniformArray(const char* name, const gl::vec2f* pointer, size_t length)
    {
        std::vector<float> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform2fvARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }


    void Shader::setUniformArray(const char* name, const gl::vec3f* pointer, size_t length)
    {
        std::vector<float> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform3fvARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }

    void Shader::setUniformArray(const char* name, const gl::vec4f* pointer, size_t length)
    {
      std::vector<float> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform4fvARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }

    void Shader::setUniformArray(const char* name, const int* pointer, size_t length)
    {
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform1ivARB(binder.location, static_cast<GLsizei>(length), pointer))
    }

    void Shader::setUniformArray(const char* name, const gl::vec2i* pointer, size_t length)
    {
        std::vector<int> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform2ivARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }

    void Shader::setUniformArray(const char* name, const gl::vec3i* pointer, size_t length)
    {
        std::vector<int> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform3ivARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }

    void Shader::setUniformArray(const char* name, const gl::vec4i* pointer, size_t length)
    {
        std::vector<int> con = flatten(pointer, length);
        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniform4ivARB(binder.location, static_cast<GLsizei>(length), &con[0]))
    }

    void Shader::setUniformArray(const char* name, const gl::mat3* pointer, size_t length)
    {
        const std::size_t matrix_size = 3 * 3;
        std::vector<float> con(matrix_size * length);
        for(size_t i = 0; i < length; ++i)
            gl::spCopyMat(pointer[i].pointer, matrix_size, &con[matrix_size * i]);

        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniformMatrix3fvARB(binder.location, static_cast<GLsizei>(length), GL_FALSE, &con[0]))
    }


    void Shader::setUniformArray(const char* name, const gl::mat4* pointer, size_t length)
    {
        const std::size_t matrix_size = 4 * 4;
        std::vector<float> con(matrix_size * length);
        for(size_t i = 0; i < length; ++i)
            gl::spCopyMat(pointer[i].pointer, matrix_size, &con[matrix_size * i]);

        Uniform binder(*this, name);
        if(binder.location != -1)
            spCheck(glUniformMatrix4fvARB(binder.location, static_cast<GLsizei>(length), GL_FALSE, &con[0]))
    }

    void Shader::bindTextures() const
    {
        auto it = m_texture_map.begin();
        for(size_t i = 0; i < m_texture_map.size(); ++i)
        {
            GLint index = static_cast<GLsizei>(i + 1);
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB + index))
            spCheck(glBindTexture(GL_TEXTURE_2D, it->second));
            spCheck(glUniform1iARB(it->first, index));
            ++it;
        }
        spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
    }

    void Shader::bind(const Shader* shader)
    {
        if(!shader_objects_supported())
            return;

        if(shader && shader->m_native_shader)
        {
            spCheck(glUseProgramObjectARB(to_GLhandle(shader->m_native_shader)))
            shader->bindTextures();
            if(shader->m_current_texture != -1)
            {
                spCheck(glUniform1iARB(shader->m_current_texture, 0))
            }
        }
        else
        {
            spCheck(glUseProgramObjectARB(0))
        }
    }

    unsigned int Shader::getCurrentTexture() const
    {
        return m_current_texture;
    }

    unsigned int Shader::getHandleGL() const
    {
        return m_native_shader;
    }

}
