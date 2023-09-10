#ifndef LABEL_H
#define LABEL_H
#include <sp/sp.h>
#include <sp/string.h>
#include <sp/gxsp/font.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/color.h>
#include <sp/gxsp/vertex.h>
#include <sp/math/rect.h>
#include <memory>

namespace sp
{
    class SP_API Label : public Drawable
    {
        public:
            typedef std::shared_ptr<Label>          Ptr;
            typedef std::shared_ptr<Label>          ConstPtr;


            ~Label();
            static Ptr          create();
            void                setCharacterSize(unsigned int charSize);
            void                setString(const String& text);
            void                setColor(const Color& color);
            void                setFont(Font::Ptr font);

            sp::vec2f           getSize() const;
            const String&       getString() const;
            void                setPosition(float x, float y) override;
            void                setLabel(String text, Font::Ptr font, unsigned charSize, const Color& color);

        private:
                                Label();
            void                refresh();
            virtual void        addDrawable(Renderer& renderer, bool max);
            void                createCharacter(unsigned int prev, unsigned int curr);
            void                insertCharacter(const vec2f& offset, const SP_Character& character);
            void                insertWhitespace(const vec2f& offset, float advance);


            const SP_Font_Map*      m_map;
            String                  m_string;
            String                  m_previous_string;
            Color                   m_color;
            std::weak_ptr<Font>     m_font;

            float                   m_ascent;
            float                   m_advance;
            unsigned int            m_char_size;
            size_t                  m_character_count;
            size_t                  m_previous_count;
            size_t                  m_traverser;
            bool                    m_refresh;
    };
}
#endif // LABEL_H
