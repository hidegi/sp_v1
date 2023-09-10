#ifndef CUSTOM_DRAWABLE_H_INCLUDED
#define CUSTOM_DRAWABLE_H_INCLUDED
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/viewport.h>
#include <functional>

namespace sp
{
    class SP_API CustomDrawable : public Drawable
    {
        public:
            typedef std::shared_ptr<CustomDrawable>             Ptr;
            typedef std::shared_ptr<const CustomDrawable>       ConstPtr;

            static Ptr create()
            {
                return Ptr(new CustomDrawable);
            }

            void setViewport(const Viewport& viewport)
            {
                m_viewport = &viewport;
                m_states.viewport = m_viewport;
            }

            void setDrawCallback(std::function<void()> fn)
            {
                //m_callback = fn;
                Drawable::setDrawCallback(fn, true);
            }

        private:
            CustomDrawable() :
                Drawable(),
                m_callback  {nullptr},
                m_viewport  {nullptr}
            {
            }
            std::function<void()>   m_callback;
            const sp::Viewport*     m_viewport;
    };
}

#endif // CUSTOM_DRAWABLE_H_INCLUDED
