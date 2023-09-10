#include <sp/gxsp/texture.h>
#include <sp/utils/helpers.h>
#include <sp/sp_controller.h>
#include <sp/spgl.h>

namespace sp
{
    namespace
    {
        SPuint64 gen_unique_id()
        {
            static SPuint64 generator = 1;
            return generator++;
        }
        unsigned int max_texture_size()
        {
            static char checked = 0;
            static int max_size = 0;
            if(!checked)
            {
                checked = 1;
                spCheck(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size));
            }
            return static_cast<unsigned int>(max_size);
        }
    }

    Texture::Texture() :
        m_size              {0, 0},
        m_tex_obj           {0},
        m_smooth            {false},
        m_repeated          {false},
        m_flipped           {false},
        m_is_fbo_attachment {false},
        m_mipmap_generated  {false},
        m_is_copy           {false},
        m_pbo_created       {false},
        m_api_id            {gen_unique_id()},
        m_pack_pbo          {0},
        m_unpack_pbo        {0}
    {
    }

    Texture::Texture(const Texture& other) :
        m_size              {other.m_size},
        m_tex_obj           {other.m_tex_obj},
        m_repeated          {other.m_repeated},
        m_flipped           {other.m_flipped},
        m_is_fbo_attachment {other.m_is_fbo_attachment},
        m_mipmap_generated  {other.m_mipmap_generated},
        m_is_copy           {true},
        m_pbo_created       {false},
        m_iformat           {other.m_iformat},
        m_format            {other.m_format},
        m_api_id            {gen_unique_id()},
        m_pack_pbo          {0},
        m_unpack_pbo        {0}
    {
    }

    Texture& Texture::operator=(const sp::Texture& other)
    {
        if(this != &other)
        {
            m_size              = other.m_size;
            m_tex_obj           = other.m_tex_obj;
            m_smooth            = other.m_smooth;
            m_repeated          = other.m_repeated;
            m_flipped           = other.m_flipped;
            m_is_fbo_attachment = other.m_is_fbo_attachment;
            m_mipmap_generated  = other.m_mipmap_generated;
            m_iformat           = other.m_iformat;
            m_format            = other.m_format;
            m_api_id            = gen_unique_id();
            m_is_copy           = true;
            m_pbo_created       = false;
            m_pack_pbo          = 0;
            m_unpack_pbo        = 0;
        }
        return *this;
    }

    Texture::~Texture()
    {
        //printf("texture deleted..\n");
        if(!Controller::active())
            return;

        if(m_tex_obj && !m_is_copy)
        {
            GLuint texture = static_cast<GLuint>(m_tex_obj);
            spCheck(glDeleteTextures(1, &texture))
        }

        if(m_pbo_created)
        {
            spCheck(glDeleteBuffersARB(1, &m_unpack_pbo))
            spCheck(glDeleteBuffersARB(1, &m_pack_pbo))
        }
    }

    unsigned int getMaxTexSize()
    {
        return max_texture_size();
    }

    bool Texture::create(unsigned int width, unsigned int height, int iformat, int format)
    {
        if(width <= 0 || height <= 0)
        {
            SP_PRINT_WARNING("cannot create texture with dimensions less-equal to zero");
            return false;
        }

        unsigned int max_size = max_texture_size();
        if(width > max_size || height > max_size)
        {
            SP_PRINT_WARNING("cannot create texture with exceeding dimensions (max size = " << max_size << ")");
            return false;
        }
        m_size = {width, height};

        m_flipped           = false;
        m_is_fbo_attachment = false;
        m_mipmap_generated  = false;

        if(!m_tex_obj)
        {
            GLuint texture = 0;
            spCheck(glGenTextures(1, &texture));
            m_tex_obj = static_cast<unsigned int>(texture);
        }

        spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
        spCheck(glTexImage2D(GL_TEXTURE_2D, 0, iformat, m_size.x, m_size.y, 0, format, GL_UNSIGNED_BYTE, NULL))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE_EXT))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE_EXT))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
        m_api_id = gen_unique_id();

        m_iformat = iformat;
        m_format  = format;



        return true;
    }

    void Texture::reset()
    {
        if(m_tex_obj && !m_is_copy)
        {
            GLuint texture = static_cast<GLuint>(m_tex_obj);
            spCheck(glDeleteTextures(1, &texture))
        }

        if(m_pack_pbo)
        {
            spCheck(glDeleteBuffersARB(GL_PIXEL_UNPACK_BUFFER_ARB, &m_pack_pbo))
            m_pack_pbo = 0;
        }

        if(m_unpack_pbo)
        {
            spCheck(glDeleteBuffersARB(GL_PIXEL_UNPACK_BUFFER_ARB, &m_unpack_pbo))
            m_unpack_pbo = 0;
        }

        m_tex_obj           = 0;
        m_size              = {0, 0};
        m_smooth            = false;
        m_repeated          = false;
        m_flipped           = false;
        m_is_fbo_attachment = false;
        m_is_copy           = false;
    }

    //loads an image located in memory..
    bool Texture::loadFromMemory(const void* data, unsigned int width, unsigned int height, const recti& area)
    {
        if(area.width == 0 || area.height == 0 ||
          ((area.left <= 0) && (area.top <= 0) && (area.width >= static_cast<int>(width)) && (area.height >= static_cast<int>(height))))
        {
            if(create(width, height))
            {
                update(reinterpret_cast<const unsigned char*>(data));
                return true;
            }

            SP_PRINT_WARNING("failed to create texture");
            return false;
        }

        recti rect = area;
        if(rect.left < 0) rect.left = 0;
        if(rect.top  < 0) rect.top  = 0;
        if(rect.left + rect.width > static_cast<int>(width))  rect.width = width - rect.left;
        if(rect.top + rect.height > static_cast<int>(height)) rect.height = height - rect.top;
        if(create(rect.width, rect.height))
        {
            const unsigned char* pixels = reinterpret_cast<const unsigned char*>(data) + 4 * (rect.left + (width * rect.top));
            spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
            for(int i = 0; i < rect.height; ++i)
            {
                spCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, rect.width, 1, m_format, GL_UNSIGNED_BYTE, pixels))
                pixels += 4 * width;
            }

            spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
            m_mipmap_generated = false;
            spCheck(glFlush())
            return true;
        }

        return false;
    }
    bool Texture::loadFromFile(const char* filename, const recti& area)
    {
        unsigned int width  = 0;
        unsigned int height = 0;
        unsigned char* data = reinterpret_cast<unsigned char*>(spHelperFileImage(filename, &width, &height, NULL, 4));
        if(!data)
        {
            SP_PRINT_WARNING("failed to load file " << filename);
            return false;
        }

        bool create = loadFromMemory(data, width, height, area);

        free(data);
        return create;
    }
    void Texture::update(const SPuint8* pixels)
    {
        update(pixels, 0, 0, m_size.x, m_size.y);
    }

    void Texture::update(const SPuint8* pixels, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        if((x + width > m_size.x) || (y + height > m_size.y))
        {
            SP_PRINT_WARNING("cannot have texture with exceeding sub texture dimensions");
            return;
        }

        if(pixels && m_tex_obj)
        {
            spCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, m_format, GL_UNSIGNED_BYTE, pixels))

            spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
            m_mipmap_generated = false;
            m_flipped = false;
            m_api_id = gen_unique_id();
            spCheck(glFlush())
        }
    }

    void Texture::update(const Texture& texture)
    {
        update(texture, 0, 0);
    }

    void Texture::update(const Texture& texture, unsigned int x, unsigned int y)
    {
        if((x + texture.m_size.x > m_size.x) || (y + texture.m_size.y > m_size.y))
        {
            SP_PRINT_WARNING("cannot have texture with exceeding sub texture dimensions");
            return;
        }

        if(!m_tex_obj || !texture.m_tex_obj)
        {
            SP_PRINT_WARNING("cannot create copy from unavailable texture object");
            return;
        }

        if(GL_EXT_framebuffer_object_supported && GL_EXT_framebuffer_blit_supported)
        {
            GLint read_fbo = 0;
            GLint draw_fbo = 0;

            spCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &read_fbo))
            spCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &draw_fbo))

            GLuint src_fbo;
            GLuint dst_fbo;
            spCheck(glGenFramebuffersEXT(1, &src_fbo))
            spCheck(glGenFramebuffersEXT(1, &dst_fbo))
            if(!src_fbo || !dst_fbo)
            {
                SP_PRINT_WARNING("cannot copy texture to framebuffer object")
                return;
            }

            spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, src_fbo))
            spCheck(glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture.m_tex_obj, 0))

            spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dst_fbo))
            spCheck(glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_tex_obj, 0))

            GLenum src_status;
            spCheck(src_status = glCheckFramebufferStatusEXT(GL_READ_FRAMEBUFFER_EXT))

            GLenum dst_status;
            spCheck(dst_status = glCheckFramebufferStatusEXT(GL_DRAW_FRAMEBUFFER_EXT))

            if((src_status != GL_FRAMEBUFFER_COMPLETE_EXT)
            || (dst_status != GL_FRAMEBUFFER_COMPLETE_EXT))
            {
                SP_PRINT_WARNING("cannot copy texture to framebuffer object");
                spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_BINDING_EXT, read_fbo))
                spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_BINDING_EXT, draw_fbo))
                spCheck(glDeleteFramebuffersEXT(1, &src_fbo))
                spCheck(glDeleteFramebuffersEXT(1, &dst_fbo))
                return;
            }

            //core..
            spCheck(glBlitFramebufferEXT(0,
                                         m_flipped ? texture.m_size.y : 0,
                                         texture.m_size.x,
                                         m_flipped ? 0: texture.m_size.y,
                                         x, y,
                                         x + texture.m_size.x,
                                         y + texture.m_size.y,
                                         GL_COLOR_BUFFER_BIT,
                                         GL_LINEAR))

            spCheck(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, read_fbo))
            spCheck(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, draw_fbo))
            spCheck(glDeleteFramebuffersEXT(1, &src_fbo))
            spCheck(glDeleteFramebuffersEXT(1, &dst_fbo))

            spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
            spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
            spCheck(glBindTexture(GL_TEXTURE_2D, 0))

            m_mipmap_generated = false;
            m_flipped = false;
            m_api_id = gen_unique_id();

            spCheck(glFlush())
        }
    }

    const vec2u& Texture::getSize() const
    {
        return m_size;
    }

    bool Texture::isSmooth() const
    {
        return m_smooth;
    }

    bool Texture::isRepeated() const
    {
        return m_repeated;
    }

    bool Texture::generateMipmap() const
    {
        if(!m_tex_obj) return false;

        if(!GL_EXT_framebuffer_object_supported)
            return false;

        GLint whichID;
        spCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID))

        spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
        spCheck(glGenerateMipmapEXT(GL_TEXTURE_2D))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR))

        spCheck(glBindTexture(GL_TEXTURE_2D, whichID))

        return m_mipmap_generated = true;
    }

    unsigned int Texture::getHandleGL() const
    {
        return m_tex_obj;
    }

    void Texture::invalidateMipmap()
    {
        if(!m_mipmap_generated) return;

        spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
        spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
        m_mipmap_generated = false;
    }

    void Texture::setFlipped(bool flip)
    {
        m_flipped = flip;
    }

    bool Texture::isFlipped() const
    {
        return m_flipped;
    }
    void Texture::bind(const Texture* texture, SP_Mapping mapping)
    {
        if(texture && texture->m_tex_obj)
        {
            spCheck(glBindTexture(GL_TEXTURE_2D, texture->m_tex_obj))
            if((mapping == Pixels) || (texture->m_flipped))
            {
                static float matrix[16] =
                {
                    1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f,
                };

                if(mapping == SP_Mapping::Pixels)
                {
                    matrix[0] = 1.f / texture->m_size.x;
                    matrix[5] = 1.f / texture->m_size.y;
                    matrix[13] = 0.f;
                }

                /*
                if(mapping == SP_Mapping::Normalized)
                {
                    matrix[0] = matrix[5] = 1.f;
                    matrix[13] = 0.f;
                }
                */

                if(texture->m_flipped)
                {
                    matrix[5]  = -matrix[5];
                    matrix[13] = static_cast<float>(texture->m_size.x) / static_cast<float>(texture->m_size.y);
                }

                spCheck(glMatrixMode(GL_TEXTURE))
                spCheck(glLoadMatrixf(matrix))
                spCheck(glMatrixMode(GL_MODELVIEW))
            }
        }
        else
        {
            spCheck(glBindTexture(GL_TEXTURE_2D, 0))
            spCheck(glMatrixMode(GL_TEXTURE))
            spCheck(glLoadIdentity())
            spCheck(glMatrixMode(GL_MODELVIEW))
        }
    }

    void Texture::setRepeated(bool repeat)
    {
        if(repeat != m_repeated)
        {
            m_repeated = repeat;
            if(m_tex_obj)
            {
                spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
                spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE_EXT))
                spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE_EXT))
            }
        }
    }

    void Texture::setSmooth(bool smooth)
    {
        if(smooth != m_smooth)
        {
            m_smooth = smooth;
            if(m_tex_obj)
            {
                spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
                spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))

                if(m_mipmap_generated)
                {
                    spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR))
                }
                else
                {
                    spCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST))
                }
            }
        }
    }

    void Texture::createPBO()
    {
        spCheck(glGenBuffersARB(1, &m_pack_pbo))
        spCheck(glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, m_pack_pbo))
        spCheck(glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, m_size.x * m_size.y * 4, NULL, GL_STREAM_DRAW_ARB))

        spCheck(glGenBuffersARB(1, &m_unpack_pbo))
        spCheck(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_unpack_pbo))
        spCheck(glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_size.x * m_size.y * 4, NULL, GL_STREAM_DRAW_ARB))
        spCheck(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0))
    }

    //a pbo is a way to transfer data between buffer and textures..
         void Texture::clearArea(int x, int y, int width, int height)
    {
        if(x < 0 || y < 0 || (x + width > m_size.x) || (y + height > m_size.x))
            return;

        GLubyte* data_write = NULL;
        GLubyte* data_read = NULL;

        if(!m_pbo_created)
        {
            createPBO();
            m_pbo_created = true;
        }
        spCheck(glPixelStorei(GL_UNPACK_ALIGNMENT,  4))
        spCheck(glPixelStorei(GL_PACK_ALIGNMENT,  4))

//===================================================================
        spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
        spCheck(glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, m_pack_pbo))
        spCheck(glGetTexImage(GL_TEXTURE_2D, 0, m_format, GL_UNSIGNED_BYTE, NULL))
        spCheck( data_read = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB))


		/*
         *  if a buffer is bound, the last parameter is treated as an offset to that buffer's data store,
         *  otherwise is a pointer to client's data store (unpacking treated relative to client's pointer)..
         */
        spCheck(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_unpack_pbo))
        spCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.x, m_size.y, m_format, GL_UNSIGNED_BYTE, NULL))


        spCheck(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_unpack_pbo))
        spCheck(glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_size.x * m_size.y * 4, NULL, GL_STREAM_DRAW_ARB))
        spCheck( data_write = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB))
        if(data_write && data_read)
        {
            int dataPitch = m_size.x * 4;


            size_t counter  = 0;
            size_t offset   = 0; //(y * width + x);
            GLubyte* start  = data_write;
            GLubyte* read   = data_read;
            GLubyte* end    = start + m_size.x * m_size.y * 4;

            /**
             *  p   ... read_data
             *  N   ... row number
             *
             *  k   defined as:
             *
             *  if(s >= a)
             *      k = n * l
             *  else
             *      k = a / s * ceil(s * n * l / a)
             *
             *  n   ... number of elements in a group
             *  l   ... number of elements in a row
             *  s   ... size of an element in basic machine units (GLubyte)
             *  a   ... GL_UNPACK_ALIGNMENT
             *
             *  advancement:
             *  p = GL_UNPACK_SKIP_PIXELS * n + GL_UNPACK_SKIP_ROWS * k
             *
             */
            for(size_t _y = 0; _y < m_size.y; _y++)
            {
                for(size_t _x = 0; _x < m_size.x; _x++)
                {
                    if(
                        (_x > x && _x <= x + width)
                    &&  (_y > x && _y <= y + height)
                       )
                    {
                        memset(data_write, 0, sizeof(char) * 4);
                    }
                    else
                    {
                        memcpy(data_write, data_read, sizeof(char) * 4);
                    }
                    data_read  += 4;
                    data_write += 4;
                }
            }

            bool unmap = false;
            spCheck(unmap = glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB))
            if(!unmap)
            {
                SP_PRINT_WARNING("failed to unmap buffer");
            }


            spCheck(unmap = glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
            if(!unmap)
            {
                SP_PRINT_WARNING("failed to unmap buffer");
            }



            spCheck(glActiveTextureARB(GL_TEXTURE0_ARB))
            spCheck(glBindTexture(GL_TEXTURE_2D, m_tex_obj))
            spCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.x, m_size.y, m_format, GL_UNSIGNED_BYTE, NULL))
        }
        else
        {
            //printf("no mapping..\n");
            bool unmap = false;
            spCheck(unmap = glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB))
            if(!unmap)
            {
                SP_PRINT_WARNING("failed to unmap buffer");
            }

            spCheck(unmap = glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB))
            if(!unmap)
            {
                SP_PRINT_WARNING("failed to unmap buffer");
            }
        }
        spCheck(glBindTexture(GL_TEXTURE_2D, 0))
        spCheck(glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0))
        spCheck(glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0))
        spCheck(glFlush())
    }
}
