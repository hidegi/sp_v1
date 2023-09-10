#ifndef TEXTURE_H
#define TEXTURE_H
#include <sp/math/rect.h>

namespace sp
{
    class SP_API Texture
    {
        public:
            enum SP_Mapping
            {
                Normalized,
                Pixels
            };

            Texture();
            Texture(const Texture&);
           ~Texture();

            Texture& operator=(const sp::Texture& other);
            bool create(unsigned int width, unsigned int height, int iformat = 0x1908, int format = 0x1908);
            bool loadFromFile(const char* filename, const recti& = recti{});
            bool loadFromMemory(const void* data, unsigned int width, unsigned int height, const recti& = recti{});
            void setRepeated(bool repeat);
            void setSmooth(bool smooth);
            void update(const SPuint8* pixels);
            void update(const SPuint8* pixels, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
            void update(const Texture& texture);
            void update(const Texture& texture, unsigned int x, unsigned int y);
            void setFlipped(bool flip);
            void clearArea(int x, int y, int width, int height);
            void createPBO();

            bool isSmooth() const;
            bool isRepeated() const;
            bool generateMipmap() const;
            bool isFlipped() const;
            const vec2u& getSize() const;
            unsigned int getHandleGL() const;
            static void bind(const Texture* texture, SP_Mapping = Normalized);
            static unsigned int getMaxTexSize();

            void reset();
        private:
            friend class Target;
            friend class Framebuffer;

            void invalidateMipmap();

        private:
            vec2u           m_size;
            unsigned int    m_tex_obj;
            bool            m_smooth;
            bool            m_repeated;
            mutable bool    m_flipped;
            bool            m_is_copy;
            bool            m_is_fbo_attachment;
            mutable bool    m_mipmap_generated;

            int             m_iformat;
            int             m_format;
            SPuint64        m_api_id;

            bool            m_pbo_created;
            unsigned int    m_pack_pbo;
            unsigned int    m_unpack_pbo;
    };
}

#endif // TEXTURE_H
