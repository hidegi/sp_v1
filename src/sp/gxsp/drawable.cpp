#include <sp/spgl.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/shader.h>
#include <sp/gxsp/texture.h>
#include <sp/gxsp/blending.h>
#include <sp/gxsp/batch_renderer.h>
#include <sp/exception.h>

namespace sp
{
    namespace
    {
        long int genUniqueID()
        {
            static long int id = 1;
            //motex locks..
            return id++;
        }


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

        void applyBlending(const Blending& b)
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
        }
    }
    Drawable::Drawable() :
        m_drawable_states   {}
    {
        m_drawable_states = std::shared_ptr<DrawableStates>(new DrawableStates);
        m_drawable_states->client   = this;
        m_drawable_states->id       = genUniqueID();
        //printf("id: %d\n", id);
    }

    Drawable::~Drawable()
    {
    }

    long int Drawable::getUniqueID() const
    {
        return genUniqueID();
    }
    Drawable::Ptr Drawable::get()
    {
        try
        {
            shared_from_this();
        }
        catch(std::bad_weak_ptr const&)
        {
            throw SP_EXCEPTION(L"drawable is not owned by any smart pointer");
        }
    }

    void Drawable::draw(const Vertex* vertices, size_t length, int width, int height, const CustomDrawStates& states,
                        bool use_tex_coord_array, const sp::Color& color)
    {
        if(!vertices || length == 0)
            return;

        spCheck(glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT))
        spCheck(glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT))

        spCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f))
        spCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))

        static Viewport viewport = {};
        viewport.setViewport(rectf{0.f, 0.f, float(width), float(height)});
        viewport.load();

        spCheck(glDisable(GL_CULL_FACE))
        spCheck(glDisable(GL_LIGHTING))
        spCheck(glDisable(GL_DEPTH_TEST))
        spCheck(glDisable(GL_ALPHA_TEST))
        spCheck(glEnable(GL_TEXTURE_2D))
        spCheck(glEnable(GL_BLEND))
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadIdentity())
        spCheck(glLoadMatrixf(states.matrix()))

        spCheck(glEnableClientState(GL_VERTEX_ARRAY))
        spCheck(glEnableClientState(GL_COLOR_ARRAY))
        if(use_tex_coord_array)
            spCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
        else
            spCheck(glDisableClientState(GL_TEXTURE_COORD_ARRAY))
        if(GL_ARB_multitexture_supported)
        {
            spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
        }
        applyBlending(states.blend_mode);
        sp::Texture::bind(states.texture);
        sp::Shader::bind(states.shader);

        const char* data = reinterpret_cast<const char*>(vertices);

        spCheck(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), data));
        spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 8))

        if(use_tex_coord_array)
            spCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12))

        //spCheck(glDrawElements(states.primitive_type, (size_t)drawable->m_indices.size(), GL_UNSIGNED_INT, (void*)(&drawable->m_indices[0])))
        spCheck(glDrawArrays(states.primitive, 0, (GLsizei)length))
        spCheck(glPopAttrib())
        spCheck(glPopClientAttrib());
        sp::Texture::bind(NULL);
        sp::Shader::bind(NULL);
    }

    void Drawable::add(const sp::Vertex& vertex)
    {
        size_t len = m_vertices.size();
        for(size_t i = 0; i < len; i++)
        {
            if(m_vertices[i] == vertex)
            {
                m_indices.push_back(static_cast<unsigned int>(i));
                return;
            }
        }

        vec2f pos = vertex.position;
        m_drawable_states->bounds.left   = std::min(m_drawable_states->bounds.left  , pos.x);
        m_drawable_states->bounds.top    = std::min(m_drawable_states->bounds.top   , pos.y);
        m_drawable_states->bounds.width  = std::max(m_drawable_states->bounds.width , pos.x);
        m_drawable_states->bounds.height = std::max(m_drawable_states->bounds.height, pos.y);

        m_indices.push_back(static_cast<unsigned int>(m_vertices.size()));
        m_vertices.push_back(vertex);
    }

    void Drawable::setDrawCallback(std::function<void()> fn, bool enable)
    {
        m_drawable_states->states.custom_draw_fn = fn;
        m_drawable_states->states.custom_draw_enable = enable;
    }

    bool Drawable::enableCustomDraw(bool enable)
    {
        if(m_drawable_states->states.custom_draw_fn)
            m_drawable_states->states.custom_draw_enable = enable;
    }


    void Drawable::setTexture(const sp::Texture& texture)
    {
        m_drawable_states->states.texture = &texture;
    }

    void Drawable::setShader(const sp::Shader& shader)
    {
        m_drawable_states->states.shader = &shader;
    }

    void Drawable::setPosition(const vec2f& pos)
    {
        setPosition(pos.x, pos.y);
    }

    void Drawable::addDrawable(Renderer& renderer, bool max)
    {
        renderer.addDrawable(m_drawable_states, max);
    }

    void Drawable::removeDrawable(Renderer& renderer)
    {
        renderer.removeDrawable(m_drawable_states);
    }
    void Drawable::setPosition(float x, float y)
    {
        m_drawable_states->position.x = x;
        m_drawable_states->position.y = y;
        m_drawable_states->update = true;
    }

    const vec2f& Drawable::getPosition() const
    {
        return m_drawable_states->position;
    }

    void Drawable::setZOrder(int level)
    {
        //not tested..
        m_drawable_states->zorder = level;
    }

    int Drawable::getZOrder() const
    {
        return m_drawable_states->zorder;
    }

    const Vertex* Drawable::getVertices() const
    {
        return &m_vertices[0];
    }

    const unsigned int* Drawable::getIndices() const
    {
        return &m_indices[0];
    }

    const Texture* Drawable::getTexture() const
    {
        return m_drawable_states->states.texture;
    }

    const Shader* Drawable::getShader() const
    {
        return m_drawable_states->states.shader;
    }

    void Drawable::setVisible(bool visible)
    {
        if(m_drawable_states->visible != visible)
           m_drawable_states->visible = visible;
    }

    bool Drawable::isVisible() const
    {
        return m_drawable_states->visible;
    }

    void Drawable::reset()
    {
        m_vertices.clear();
        m_indices.clear();

        m_drawable_states->bounds.left   =
        m_drawable_states->bounds.top    =
        m_drawable_states->bounds.width  =
        m_drawable_states->bounds.height = 0.f;

        m_drawable_states->position.x    =
        m_drawable_states->position.y    = 0.f;

        m_drawable_states->zorder = 0;
        m_drawable_states->states = {};
    }

    rectf Drawable::getLocalBounds() const
    {
        float width  = m_drawable_states->bounds.width;
        float height = m_drawable_states->bounds.height;
        return rectf{0.f, 0.f, width, height};
    }

    rectf Drawable::getGlobalBounds() const
    {
        rectf local = getLocalBounds();
        local.left  = m_drawable_states->position.x;
        local.top   = m_drawable_states->position.y;
        return local;
    }
}
