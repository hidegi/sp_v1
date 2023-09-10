#include <sp/gxsp/framebuffer.h>
#include <sp/gxsp/transformable.h>
#include <sp/sp_controller.h>
#include <sp/spgl.h>

namespace sp
{
    namespace
    {
        int max_samples()
        {
            /*
            static int  samples = 0;
            static char checked = 0;
            if(!checked)
            {
                checked = 1;
                GLint msaa = 0;
                spCheck(glGetIntegerv(GL_MAX_SAMPLES_EXT, &msaa))
                samples = static_cast<int>(msaa);
            }
            return samples;
            */
            return 4;
        }

        int max_color_attachments()
        {
            static int  attachments = 0;
            static char checked = 0;
            if(!checked)
            {
                checked = 1;
                GLint _attachments = 0;
                spCheck(glGetIntegerv(GL_MAX_SAMPLES_EXT, &_attachments))
                attachments = static_cast<int>(_attachments);
            }
            return attachments;
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
    }

    Framebuffer::Framebuffer() :
        m_fbo_obj               {0},
        m_fbo_msaa_obj          {0},
        m_rbo_obj               {0},
        m_rbo_msaa_color_obj    {0},
        m_rbo_msaa_depth_obj    {0},
        m_color_buffer          {nullptr},
        m_samples               {0},
        m_texture               {0},
        m_size                  {0, 0}
    {
    }

    Framebuffer::~Framebuffer()
    {
        deleteBuffers();
    }

    const sp::Texture& Framebuffer::getColorTexture()
    {
        return m_color_texture;
    }
    bool Framebuffer::create(unsigned int width, unsigned int height, bool multisample)
    {
        if(width <= 0 || height <= 0)
        {
            SP_PRINT_WARNING("cannot have size set less-equal zero");
            return false;
        }

        deleteBuffers();
        //printf("max attachments: %d\n", max_color_attachments());
        m_color_buffer = reinterpret_cast<unsigned char*>(calloc(width * height * 4, sizeof(unsigned char)));
        m_color_texture.create(width, height);
        m_size = {width, height};

        spCheck(glGenFramebuffersEXT(1, &m_fbo_obj))
        spCheck(glGenRenderbuffersEXT(1, &m_rbo_obj))
        //spCheck(glGenTextures(1, &m_texture))
        m_samples = multisample ? max_samples() : 0;

        GLint read_framebuffer = 0;
        GLint draw_framebuffer = 0;

        spCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &read_framebuffer))
        spCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &draw_framebuffer))

        spCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo_obj))

        /*
        m_dummy_texture.create(width, height);
        m_dummy_texture.generateMipmap();
        m_dummy_texture.m_is_fbo_attachment = true;
        */
        m_color_texture.create(width, height);
        m_color_texture.generateMipmap();
        m_color_texture.setRepeated(true);
        m_color_texture.m_is_fbo_attachment = true;

        /*
        spCheck(glBindTexture(GL_TEXTURE_2D, m_texture))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE))
        spCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL))
        */
        ///m_texture.create(width, height);
        /*
         *  class texture must have reset()..
         */
        //spCheck(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, /*m_texture*/ m_dummy_texture.getHandleGL(), 0))
        spCheck(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + 2, GL_TEXTURE_2D, /*m_texture*/ m_color_texture.getHandleGL(), 0))

        spCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_rbo_obj))
        spCheck(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height))
        spCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_rbo_obj))

        bool status = isComplete();
        if(m_samples > 0)
        {
            spCheck(glGenFramebuffersEXT(1, &m_fbo_msaa_obj))
            spCheck(glGenRenderbuffersEXT(1, &m_rbo_msaa_color_obj))
            spCheck(glGenRenderbuffersEXT(1, &m_rbo_msaa_depth_obj))
            spCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo_msaa_obj))


            //color attachment..
            spCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_rbo_msaa_color_obj))
            spCheck(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, m_samples, GL_RGBA8, width, height))
            spCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + 2, GL_RENDERBUFFER_EXT, m_rbo_msaa_color_obj))

            //depth attachment..
            spCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_rbo_msaa_depth_obj))
            spCheck(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, m_samples, GL_DEPTH_COMPONENT, width, height))
            spCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_rbo_msaa_depth_obj))

            status = status && isComplete();
        }
        spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, read_framebuffer))
        spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, draw_framebuffer))

        spCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0))
        spCheck(glBindTexture(GL_TEXTURE_2D, 0))

        return status;
    }

    void Framebuffer::bind()
    {
        /*
        static unsigned int attachments[] = {GL_BACK_LEFT};
        spCheck(glDrawBuffersEXT(1, attachments));
        */

        if(m_samples > 0)
        {
            spCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo_msaa_obj))
        }
        else
        {
            spCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo_obj))
            /*
            spCheck(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))
            spCheck(glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))
            */
        }
        //whatever is bound, all drawn to color attachment0 + n..
        spCheck(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))
    }


    static void applyBlending(const Blending& b)
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

        if(GL_EXT_blend_minmax_supported && GL_EXT_blend_subtract_supported)
        {
            if(GL_EXT_blend_equation_separate_supported)
            {
                spCheck(glBlendEquationSeparateEXT(
                    translateBlendEquation(b.colorEquation),
                    translateBlendEquation(b.alphaEquation)
                ))
            }
            else
            {
                spCheck(glBlendEquationEXT(translateBlendEquation(b.colorEquation)))
            }
        }
    }

    void Framebuffer::destroy()
    {
        deleteBuffers();
    }

    void Framebuffer::draw(
        const Vertex* vertices,
        size_t length,
        int primitive_type,
        const vec2f& offset,
        const vec2f& scale,
        float point_size,
        Blending blending,
        const sp::Texture* texture,
        const sp::Shader* shader)
    {
        spCheck(glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT))
        spCheck(glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT))

        if(GL_ARB_multitexture_supported)
        {
            spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
        }

        spCheck(glPointSize(1.f));
        spCheck(glDisable(GL_CULL_FACE))
        spCheck(glDisable(GL_LIGHTING))
        spCheck(glDisable(GL_DEPTH_TEST))
        spCheck(glDisable(GL_ALPHA_TEST))
        spCheck(glEnable(GL_TEXTURE_2D))
        spCheck(glEnable(GL_BLEND))
        spCheck(glEnable(GL_SCISSOR_TEST))
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadIdentity())
        spCheck(glPushMatrix())
        spCheck(glEnableClientState(GL_VERTEX_ARRAY))
        spCheck(glEnableClientState(GL_COLOR_ARRAY))
        spCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
        applyBlending(blending);
        sp::Texture::bind(texture, sp::Texture::SP_Mapping::Pixels);
        sp::Shader::bind(shader);


        static Viewport viewport;

        viewport.setViewport(sp::rectf{0.f, 0.f, m_size.x, m_size.y});
        viewport.load();

        if(!vertices || length == 0)
            return;

        static Transformable trm;
        trm.setPosition(offset);
        trm.setScale(scale);
        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glLoadMatrixf(trm.getMatrix()()))


        if(primitive_type == GL_POINTS)
		{
			spCheck(glEnable(GL_LINE_SMOOTH));
			spCheck(glEnable(GL_POINT_SMOOTH));
			spCheck(glEnable(GL_POLYGON_SMOOTH));
			spCheck(glPointSize(point_size))

			if(GL_ARB_point_sprite_supported)
			{
				spCheck(glEnable(GL_POINT_SPRITE_ARB))
				spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE))
			}
		}

        const char* data = reinterpret_cast<const char*>(vertices);
        spCheck(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), data));
        spCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 8))
        spCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12))
        spCheck(glDrawArrays(primitive_type, 0, length))

        if(primitive_type == GL_POINTS)
		{
			spCheck(glDisable(GL_POINT_SPRITE_ARB))
			spCheck(glPointSize(1.f))
			spCheck(glDisable(GL_LINE_SMOOTH));
			spCheck(glDisable(GL_POINT_SMOOTH));
			spCheck(glDisable(GL_POLYGON_SMOOTH));
			if(GL_ARB_point_sprite_supported)
			{
				spCheck(glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE));
			}
		}

        spCheck(glMatrixMode(GL_MODELVIEW))
        spCheck(glPopMatrix())

        if(GL_ARB_multitexture_supported)
        {
            spCheck(glClientActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
        }

        sp::Texture::bind(NULL);
        sp::Shader::bind(NULL);

        spCheck(glPopClientAttrib())
		spCheck(glPopAttrib())
    }

    //display(int texture);

    /*
     *  traditional buffers are GL_FRONT_LEFT, GL_BACK_LEFT or GL_FRONT_BACK_LEFT, etc..
     *  once an fbo is bound, glDrawBuffer(s)/glReadBuffer(s) will only accept attachment points
     *  of some fbo..
     */
    void Framebuffer::display()
    {
        /*
        GLint read_framebuffer = 0;
        GLint draw_framebuffer = 0;

        spCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &read_framebuffer))
        spCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &draw_framebuffer))
        */
        /*
        static unsigned int attachments[2] = {GL_NONE, GL_COLOR_ATTACHMENT0_EXT + 2};
        spCheck(glDrawBuffersEXT(2, attachments));
        */
       // spCheck(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 2));

        if(m_samples > 0)
        {
            //read from multsample fbo color attachment0 + n..
            spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo_msaa_obj))
            spCheck(glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))

            //draw to normal fbo color attachment0 + n..
            spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo_obj))
            spCheck(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))
            /*
            spCheck(glBlitFramebufferEXT(0, m_size.y, m_size.x, 0, 0, 0, m_size.x, m_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR))
            spCheck(glBlitFramebufferEXT(0, m_size.y, m_size.x, 0, 0, 0, m_size.x, m_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST))
            */

            //blit..
            spCheck(glBlitFramebufferEXT(0, 0, m_size.x, m_size.y, 0, 0, m_size.x, m_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR))
            spCheck(glBlitFramebufferEXT(0, 0, m_size.x, m_size.y, 0, 0, m_size.x, m_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST))

        }
        //spCheck(glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + 2))
        /*
        spCheck(glBindTexture(GL_TEXTURE_2D, m_texture))
        spCheck(glGenerateMipmapEXT(GL_TEXTURE_2D))
        spCheck(glBindTexture(GL_TEXTURE_2D, 0))
        */
        //sp::Texture::bind(m_color_texture, sp::Texture::SP_Mapping::Normalized);
        m_color_texture.generateMipmap();

        //spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, read_framebuffer))
        //spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, draw_framebuffer))
        spCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0))
        spCheck(glDrawBuffer(GL_LEFT));
        spCheck(glReadBuffer(GL_LEFT));
    }

    void Framebuffer::deleteBuffers()
    {
        if(m_color_buffer)
        {
            free(m_color_buffer);
            m_color_buffer = nullptr;
        }

        if(!Controller::active())
            return;

        m_color_texture.reset();
        //m_dummy_texture.reset();

        if(m_rbo_msaa_color_obj)
        {
            spCheck(glDeleteRenderbuffersEXT(1, &m_rbo_msaa_color_obj))
            m_rbo_msaa_color_obj = 0;
        }

        if(m_rbo_msaa_depth_obj)
        {
            spCheck(glDeleteRenderbuffersEXT(1, &m_rbo_msaa_depth_obj))
            m_rbo_msaa_depth_obj = 0;
        }

        if(m_fbo_msaa_obj)
        {
            spCheck(glDeleteFramebuffersEXT(1, &m_fbo_msaa_obj))
            m_fbo_msaa_obj = 0;
        }

        /*
        if(m_texture)
        {
            spCheck(glDeleteTextures(1, &m_texture))
            m_texture = 0;
        }
        */


        if(m_fbo_obj)
        {
            spCheck(glDeleteFramebuffersEXT(1, &m_fbo_obj))
            m_fbo_obj = 0;
        }
    }

    unsigned int Framebuffer::getTexHandleGL()
    {
        //return m_texture;
        return m_color_texture.getHandleGL();
    }

    sp::Texture Framebuffer::copyColorToTexture()
    {
        Texture texture;
        texture.create(m_size.x, m_size.y);
        texture.update(getColorBuffer());
        Texture::bind(NULL);
        return texture;
    }

    void Framebuffer::copyColor(unsigned int dstID, int x, int y, int width, int height)
    {
        if(!width)  width  = m_size.x;
        if(!height) height = m_size.y;

        GLuint srcID = (m_samples == 0) ? m_fbo_obj : m_fbo_msaa_obj;
        spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, srcID))
        spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dstID))
        spCheck(glBlitFramebufferEXT(0, 0, m_size.x, m_size.y, x, y, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR))
    }

    const SPuint8* Framebuffer::getColorBuffer()
    {
        if(m_samples > 0)
        {
            copyColor(m_fbo_obj);
        }
        spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo_obj))
        spCheck(glReadPixels(0, 0, m_size.x, m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, m_color_buffer))
        return m_color_buffer;
    }
    bool Framebuffer::isComplete()
    {
        GLenum status = 0;
        spCheck(status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
        switch(status)
        {
            case GL_FRAMEBUFFER_COMPLETE_EXT:
                return true;

            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            {
                SP_PRINT_WARNING("no image has been attached to framebuffer");
                return false;
            }

            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            {
                SP_PRINT_WARNING("framebuffer incomplete, due to draw buffer");
                return false;
            }

            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            {
                SP_PRINT_WARNING("framebuffer incomplete, due to read buffer");
                return false;
            }

            default:
            {
                SP_PRINT_WARNING("framebuffer incomplete, due to unknown reason");
                break;
            }
        }
        return false;
    }

    void Framebuffer::clear(const sp::Color& color)
    {
        spCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
        spCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    }
    void Framebuffer::clearArea(int t, int l, int w, int h)
    {
        spCheck(glEnable(GL_SCISSOR_TEST))
        //spCheck(glViewport(t, l, w, h))
        spCheck(glScissor(t, l, w, h))

        spCheck(glClearColor(1.f, 0.f, 0.f, 1.f))
        spCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
        spCheck(glDisable(GL_SCISSOR_TEST))
    }
    void Framebuffer::clearArea(const vec2i& tl, const vec2i& wh)
    {
        clearArea(tl.x, tl.y, wh.x, wh.y);
    }
    void Framebuffer::clearArea(const recti& area)
    {
        clearArea(area.left, area.top, area.width, area.height);
    }
}
