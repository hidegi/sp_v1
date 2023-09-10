#ifndef ATLAS_H
#define ATLAS_H


namespace sp
{
    class Atlas
    {
        public:
                                Atlas();
                                Atlas(unsigned int dim);

            void                update(unsigned int width, unsigned int height, unsigned int id, const void* pixels);
            rectf               getTexCoords(unsigned int id, bool normalized);
            const Texture*      getTexture() const;
            size_t              getTextureCount();
            void                removeTexture(unsigned int id);
            vec2u               getSize(unsigned int id);
            vec2u               getAtlasSize();

        private:
            struct SP_Texture
            {
                SP_Texture*     nexts[2];
                vec2u           tex_coords;
                vec2u           dimensions;
                unsigned int    id;
            };

            void                init();
            SP_Texture*         m_traverser;

    };
}
#endif // ATLAS_H
