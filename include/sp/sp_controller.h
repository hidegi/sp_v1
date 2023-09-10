#ifndef SP_GLFW_CONTROLLER_H
#define SP_GLFW_CONTROLLER_H
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
//#include <sp/gxsp/render_target.h>

#include <sp/event/subject.h>
#include <sp/event/event_ex.h>
#include <sp/event/observer.h>
//#include <sp/ecs/spec.h>
//#include <sp/ecs/component_pool.h>
#include <sp/ecs/machine.h>

#include <sp/gxsp/batch_renderer.h>
#include <sp/gxsp/drawable.h>
#include <sp/gxsp/font.h>

#include <sp/sp.h>
#include <sp/sp_event.h>
#include <sp/math/rect.h>
#include <sp/gxsp/viewport.h>
#include <sp/gxsp/color.h>

/*
 *  controller:
 *      | switch_to_state()
 *      |-- state
 *            |-- gui
 *            |-- map
 *                 | load_map() / delete_map()
 *                 |-- datch
 *
 *
 */

namespace sp
{
    class Listener;

    class Subscription
    {
        public:
                                Subscription() = default;
                               ~Subscription() = default;

            void                addListener(Listener* l);
            void                removeListener(Listener* l);
            void                broadcast(SP_Detail* detail);

        private:
            std::vector<Listener*>      m_listeners;
    };

    //wrapper class for every backend unit..
    class SP_API  Controller final
    {
        public:
            typedef std::shared_ptr<Controller>             Ptr;
            typedef std::shared_ptr<const Controller>       ConstPtr;

            Controller&         operator=(const Controller&) = delete;
            Controller&         operator=(Controller&&)      = delete;
           ~Controller();

            const Viewport&     getViewport() const;
            static bool         create(const SP_Config& = {});
            static Ptr          get();

            Subject::Ptr        getSubject();
            Machine::Ptr        getContainer();
            Machine::ConstPtr   getContainer() const;

            static Font::Ptr    getDefaultFont();

            void                setUserEvent();
            void                setFramebufferPosition(const vec2f& pos);
            void                setFramebufferPosition(float x, float y);

            unsigned int        getFramebufferTexHandleGL() const;
            void                pushStates();
            void                popStates();
            //void                draw(const Drawable& draw, const States& states = States::DEFAULT);
            void                setDrawCallback(std::function<void()> fn);

            void                setPostProcessShader(const sp::Shader* shader);
            bool                running() const;
            void                setClearColor(const Color& color);
            void                setResizable(bool resize);
            /*
            void                beginDraw();
            void                endDraw();
            */

            void                draw(const Drawable::Ptr);
            void                terminate();
            void                subscribe(SP_SystemEvent event, Listener* listener);
            void                unsubscribe(SP_SystemEvent event, Listener* listener);

            void                mainLoop();
            static const void*  getHandle();
            static bool         active();


            vec2f               mapPixelsToCoords(const vec2i& point);
            vec2i               mapCoordsToPixels(const vec2f& point);

            void                addDrawable(const Drawable::Ptr primitive, bool set_max = false);
            void                removeDrawable(const Drawable::Ptr primitive);
            void                setImageIcon(unsigned char* pixels);
            void                loadImageIcon(const char* path);

            Observer::Ptr       createObserver();


//====================================================================================
//  component machine interface..
//====================================================================================
            template<typename C>
            ComponentMap<C>& iterate()
            {
                return getContainer()->iterate<C>();
            }

            entity_type createEntity()
            {
                return getContainer()->create();
            }

            template<typename C, typename ... Args>
            void emplace(entity_type entity, Args&& ... args)
            {
                getContainer()->emplace<C>(entity, std::forward<Args>(args)...);
            }

            template<typename C, typename ... Args>
            void assign(entity_type entity, Args&& ... args)
            {
                getContainer()->assign<C>(entity, std::forward<Args>(args)...);
            }

            template<typename C>
            void remove(entity_type entity)
            {
                getContainer()->remove<C>(entity);
            }

            template<typename C>
            SP_CONSTEXPR bool has(entity_type entity)
            {
                return getContainer()->has<C>(entity);
            }

            template<typename C>
            C& getComponent(entity_type entity)
            {
                return getContainer()->getComponent<C>(entity);
            }


            bool valid(entity_type entity) const noexcept
            {
                return getContainer()->valid(entity);
            }


        private:
            friend class System;
            friend class Component;
            friend class Event;

            void                dispatch(SP_Detail* detail);
            //void                dispatch(UserEvent* event);
        public:
            template<typename T, typename ... Args>
            void dispatch(long int event, Args&& ... args)
            {
                T detail((SP_SystemEvent)event, std::forward<Args>(args)...);
                dispatch(&detail);
            }

            /*
            template<typename T, typename ... Args>
            void dispatch(UserEvent* event, Args&& ... args)
            {
            }
            */

        private:
                typedef std::function<void(SP_Detail*)>	Function;

				void		init();

				/**< can all be some Args&& thing.. >**/
				/**< window events.. */
				void		fnWindowPos(int xpos, int ypos);
				void		fnWindowSize(int width, int height);
				void		fnWindowClose();
				void		fnWindowRefresh();
				void		fnWindowFocus(int focused);
				void		fnWindowIconify(int iconified);
				void		fnWindowMaximize(int maximized);
				void		fnWindowFramebufferSize(int width, int height);
				void		fnWindowContentScale(int width, int height);

				/**< input events.. */
				void		fnKey(int key, int scancode, int action, int mods);
				void		fnChar(unsigned int codepoint);
				void		fnCharMods(unsigned int codepoint, int mods);
				void		fnMouseButton(int button, int action, int mods);
				void		fnCursorPos(double xpos, double ypos);
				void		fnCursorEnter(int entered);
				void		fnScroll(double xoffset, double yoffset);
				void		fnDrop(int pathCount, const char* paths[]);

        private:
                                Controller();
            static bool         initController(const SP_Config& config);
            void                setDefaultCallbacks();
            //void                resetView(const sp::rectf& rect);

            std::chrono::steady_clock::time_point   m_lastUpdateTime;

            vec2f               m_center;
            vec2i               m_size;


            vec2f               m_lastMousePos;

            std::unordered_map<long int, Subscription>          m_subscriptions;
            std::unordered_map<SPint32, SP_SystemEvent>         m_eventContainer;

            static void*        m_window;
            static void*        m_videoMode;
            static void**       m_monitors;
            static void*        m_primary;

            std::function<void()>           m_draw_callback;

            //Target              m_target;
            Viewport            m_viewport;
            static bool         m_running;
            bool                m_window_active;
            bool                m_updateView{true};
            bool                m_updateMatrix{true};
            Color               m_clear_color;

            bool                m_resizable;
            Renderer            m_renderer;

            sp::Observer*    m_listener;

            static long int     generator;
    };
}

#endif // CONTROLLER_H
