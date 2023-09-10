#include <sp/gxsp/render_target.h>
#include <sp/sp_controller.h>
#include <sp/spgl.h>

namespace sp
{
    //test zone..
    namespace
    {
        SPuint32 translateBlendFactor(Blending::SP_Factor factor)
        {
            switch(factor)
            {
                case Blending::Zero:                    return GL_ZERO;
                case Blending::One:                     return GL_ONE;
                case Blending::SrcColor:                return GL_SRC_COLOR;
                case Blending::OneMinusSrcColor:        return GL_ONE_MINUS_SRC_COLOR;
                case Blending::DstColor:                return GL_DST_COLOR;
                case Blending::OneMinusDstColor:        return GL_ONE_MINUS_DST_COLOR;
                case Blending::SrcAlpha:                return GL_SRC_ALPHA;
                case Blending::OneMinusSrcAlpha:        return GL_ONE_MINUS_SRC_ALPHA;
                case Blending::DstAlpha:                return GL_DST_ALPHA;
                case Blending::OneMinusDstAlpha:        return GL_ONE_MINUS_DST_ALPHA;
            }

            return GL_ZERO;
        }

        SPuint32 translateBlendEquation(Blending::SP_Equation eq)
        {
            switch(eq)
            {
                case Blending::Add:             return GL_FUNC_ADD_EXT;
                case Blending::Subtract:        return GL_FUNC_SUBTRACT_EXT;
                case Blending::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT_EXT;
            }

            return GL_FUNC_ADD_EXT;
        }
    }
    Target::Target() :
        m_default_view  {},
        m_custom_view   {},
        m_cache         {},
        m_id            {1}
    {
        m_cache.gl_states_set = false;
    }

    Target::~Target()
    {
    }

    void Target::clear(const Color& color)
    {
        if(Controller::active())
        {
            //applyTexture(NULL);
            spCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f))
            spCheck(glClear(GL_COLOR_BUFFER_BIT))
        }
    }

    void Target::setViewport(const rectf& viewport)
    {
        m_custom_view.setViewport(viewport);
        m_cache.viewport_change = true;
    }

    const Viewport& Target::getViewport() const
    {
        return m_custom_view;
    }

    const Viewport& Target::getDefaultViewport() const
    {
        return m_default_view;
    }

    void Target::draw(const Drawable& drawable, const States& states)
    {
        drawable.draw(*this, states);
    }

    //an SFML equivalent..
    void Target::draw(const Vertex* vertices, size_t length, int type, const States& states)
    {
        if(!vertices || length == 0)
            return;

        if(Controller::active())
        {
            bool enable_cache = (length <= SP_Cache::CACHE_SIZE);
            if(enable_cache)
            {
                for(size_t i = 0; i < length; i++)
                {
                    Vertex& vertex      = m_cache.vertex_cache[i];
                    vertex.position     = states.matrix * vertices[i].position;
                    vertex.color        = vertices[i].color;
                    vertex.texCoords    = vertices[i].texCoords;
                }
            }
            setupDraw(enable_cache, states);
            bool use_client_texcoords = (states.texture || states.shader);

            if(!m_cache.enable || (use_client_texcoords != m_cache.use_client_texcoords))
            {
                if(use_client_texcoords)
                {
                    spCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
                }
                else
                {
                    spCheck(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
                }
            }

            if(!m_cache.enable || !enable_cache || !m_cache.use_vertex_cache)
            {
                const char* data = reinterpret_cast<const char*>(enable_cache ? m_cache.vertex_cache : vertices);

                spCheck(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), data))
                spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 8))

                if(use_client_texcoords)
                {
                    spCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12))
                }
            }
            else if(use_client_texcoords && !m_cache.use_client_texcoords)
            {
                const char* data = reinterpret_cast<const char*>(m_cache.vertex_cache);
                spCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12))
            }

            drawMesh(type, 0, length);
            cleanupDraw(states);

            m_cache.use_vertex_cache = enable_cache;
            m_cache.use_client_texcoords = use_client_texcoords;
        }
    }

    void Target::resetStatesGL()
    {
        static bool shader_available = Shader::shader_objects_supported();
        //static bool vbo_supported = VBO::vbo_supported();

        if(Controller::active())
        {
            if(GL_ARB_multitexture_supported)
            {
                spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
                spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
            }

            spCheck(glDisable(GL_CULL_FACE))
            spCheck(glDisable(GL_LIGHTING))
            spCheck(glDisable(GL_DEPTH_TEST))
            spCheck(glDisable(GL_ALPHA_TEST))
            spCheck(glEnable(GL_TEXTURE_2D))
            spCheck(glEnable(GL_BLEND))
            spCheck(glMatrixMode(GL_MODELVIEW))
            spCheck(glLoadIdentity())
            spCheck(glEnableClientState(GL_VERTEX_ARRAY))
            spCheck(glEnableClientState(GL_COLOR_ARRAY))
            spCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
            m_cache.gl_states_set = true;

            applyBlending(BlendAlpha);
            applyTexture(NULL);
            if(shader_available)
                applyShader(NULL);
            m_cache.use_client_texcoords = true;
            m_cache.use_vertex_cache = false;
            setViewport(rectf{m_custom_view.getOrigin(), m_custom_view.getSize()});
            m_cache.enable = true;
        }
    }

    void Target::pushStatesGL()
    {
        if(Controller::active())
        {
            spCheck(;)
            spCheck(glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS))
            spCheck(glPushAttrib(GL_ALL_ATTRIB_BITS))
            spCheck(glMatrixMode(GL_MODELVIEW))
            spCheck(glPushMatrix())
            spCheck(glMatrixMode(GL_PROJECTION))
            spCheck(glPushMatrix())
            spCheck(glMatrixMode(GL_TEXTURE))
            spCheck(glPushMatrix())
            spCheck(glMatrixMode(GL_MODELVIEW))
        }
        resetStatesGL();
    }

    void Target::popStatesGL()
    {
        if(Controller::active())
        {
            spCheck(glMatrixMode(GL_PROJECTION))
            spCheck(glPopMatrix())
            spCheck(glMatrixMode(GL_MODELVIEW))
            spCheck(glPopMatrix())
            spCheck(glMatrixMode(GL_TEXTURE))
            spCheck(glPopMatrix())
            spCheck(glMatrixMode(GL_MODELVIEW))
            spCheck(glPopClientAttrib())
            spCheck(glPopAttrib())
        }
    }

    void Target::initialize()
    {
        m_default_view.setViewport(rectf{0.f, 0.f, static_cast<float>(getSize().x), static_cast<float>(getSize().y)});
        m_custom_view = m_default_view;
        m_cache.gl_states_set = false;
        m_id = 1;
    }

    void Target::applyCurrentView()
    {
        if(Controller::active())
        {
            m_custom_view.load();
            m_cache.viewport_change = false;
        }
    }

    void Target::applyBlending(const Blending& b)
    {
        if(GL_EXT_blend_func_separate_supported)
        {
            spCheck(glBlendFuncSeparateEXT(
                translateBlendFactor(b.colorSrcFactor),
                translateBlendFactor(b.colorDstFactor),
                translateBlendFactor(b.alphaSrcFactor),
                translateBlendFactor(b.alphaDstFactor)
            ))
        }
        else
        {
            spCheck(glBlendFunc(
                translateBlendFactor(b.colorSrcFactor),
                translateBlendFactor(b.colorDstFactor)
            ))
        }

        spCheck(glBlendEquationSeparateEXT(
            translateBlendEquation(b.colorEquation),
            translateBlendEquation(b.alphaEquation)
        ))

        //spCheck(glBlendEquationEXT(translateBlendEquation(b.colorEquation)))

        m_cache.last_blend_mode = b;
    }

    void Target::applyTransform(const mat& m)
    {
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadMatrixf(m()))
    }

    void Target::applyTexture(const Texture* texture)
    {
        Texture::bind(texture, Texture::Pixels);
        m_cache.last_texture_id = texture ? texture->m_api_id : 0;
    }

    void Target::applyShader(const Shader* shader)
    {
        Shader::bind(shader);
    }

    void Target::setupDraw(bool cache, const States& states)
    {
        if(!m_cache.gl_states_set)
            resetStatesGL();
        if(cache)
        {
            if(!m_cache.enable || !m_cache.use_vertex_cache)
                spCheck(glLoadIdentity())
        }
        else
        {
            applyTransform(states.matrix);
        }

        if(!m_cache.enable || m_cache.viewport_change)
            applyCurrentView();


        if(!m_cache.enable || (states.blending != m_cache.last_blend_mode))
            applyBlending(states.blending);

        if(!m_cache.enable || (states.texture && states.texture->m_is_fbo_attachment))
        {
            applyTexture(states.texture);
        }
        else
        {
            SPuint64 texture_id = states.texture ? states.texture->m_api_id : 0;
            if(texture_id != m_cache.last_texture_id)
            {
                applyTexture(states.texture);
            }
        }

        if(states.shader)
            applyShader(states.shader);
    }

    void Target::drawMesh(int type, size_t first, size_t length)
    {
        if(type < 1 || type > 7)
            return;

        static const GLenum modes[7] =
        {
            GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
            GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS
        };

        GLenum mode = modes[type - 1];
        spCheck(glDrawArrays(mode, static_cast<GLint>(first), static_cast<GLsizei>(length)))
    }

    void Target::cleanupDraw(const States& states)
    {
        if(states.shader)
            applyShader(NULL);
        if(states.texture && states.texture->m_is_fbo_attachment)
            applyTexture(NULL);
        m_cache.enable = true;
    }

    void Target::setSurfaceSize(unsigned int width, unsigned int height)
    {
        m_size.x = width;
        m_size.y = height;
    }

    void Target::setSurfaceSize(vec2u dim)
    {
        setSurfaceSize(dim.x, dim.y);
    }

    vec2u Target::getSize() const
    {
        return m_size;
    }
}
