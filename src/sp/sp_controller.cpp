#include <sp/sp_controller.h>
#include <sp/spgl.h>
#include <sp/controller.h>
#include <sp/sp_listener.h>
#include <sp/utils/optional.h>
#include <sp/utils/duration.h>
#include <sp/utils/stopwatch.h>
#include <sp/utils/timer.h>
#include <sp/gxsp/sprite.h>
#include <sp/gxsp/texture.h>
#include <sp/utils/shared_mutex.h>
#include <sp/default.h>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace sp
{
    namespace
    {
        static std::shared_ptr<Font> default_font = nullptr;
        static float matrix[16] =
        {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f
        };

        bool updateMatrix = true;

        static sp::shared_mutex                 motex;
        static std::shared_ptr<sp::Subject>     s_subject   = std::make_shared<sp::Subject>();
        static std::shared_ptr<sp::Machine>     s_container = std::make_shared<sp::Machine>();

        static std::shared_ptr<Controller>      instance = nullptr;
    }
    bool    Controller::m_running = false;
    void*   Controller::m_window    {nullptr};
    void*   Controller::m_videoMode {nullptr};
    void**  Controller::m_monitors  {nullptr};
    void*   Controller::m_primary   {nullptr};

    long int Controller::generator = (long int)sp::SP_SystemEvent::SystemEventCount;

    Font::Ptr Controller::getDefaultFont()
    {
        static const void* font = NULL;
        if(!font)
        {
            font = loadDefaultFont();
            default_font = Font::create();
            default_font->loadFromMemory(font);
        }

        return default_font;
    }
    void Listener::setSystemCallback(SP_SystemEvent event, std::function<void(SP_Detail*)> fn)
    {
        m_callbacks[event].push_back(fn);
    }

    void Listener::notify(SP_Detail* detail) const
    {
        auto it = m_callbacks.find(detail->event);
        if(it != m_callbacks.end())
        {
            for(auto fn : it->second) fn(detail);
        }
    }


    void Subscription::addListener(Listener* l)
    {
        auto compare = [&](Listener* ls) {return ls == l;};
        auto it = std::find_if(m_listeners.begin(), m_listeners.end(), compare);
        if(it == m_listeners.end())
            m_listeners.push_back(l);
    }

    void Subscription::removeListener(Listener* l)
    {
        auto compare = [&](Listener* ls) {return ls == l;};
        auto it = std::find_if(m_listeners.begin(), m_listeners.end(), compare);
        if(it != m_listeners.end())
            m_listeners.erase(it);
    }

    void Subscription::broadcast(SP_Detail* detail)
    {
        for(auto listener : m_listeners)
            listener->notify(detail);
    }

    void Controller::subscribe(SP_SystemEvent event, Listener* listener)
    {
        m_subscriptions[event].addListener(listener);
    }

    void Controller::unsubscribe(SP_SystemEvent event, Listener* listener)
    {
        auto it = m_subscriptions.find(event);
        if(it != m_subscriptions.end())
            it->second.removeListener(listener);
    }

    Subject::Ptr Controller::getSubject()
    {
#if SP_COMPILE_VERSION >= 17
                std::shared_lock readGuard(motex);
#else
                s_lock readGuard(motex);
#endif
        return s_subject;
    }

    bool Controller::create(const SP_Config& config)
    {
        if(!initController(config)) return false;
        if(!instance)
        {
            instance = Ptr (new Controller);
            int width  = 0;
            int height = 0;
            glfwGetWindowSize(static_cast<GLFWwindow*>(instance->m_window), &width, &height);
            instance->m_size = {width, height};
            /*
            instance->m_target.initialize();
            instance->m_target.setSurfaceSize(width, height);
            instance->m_target.setViewport(rectf{0, 0, (float)width, (float)height});
            */
            m_running = true;
            instance->m_window_active   = true;
            instance->m_draw_callback   = nullptr;
            instance->m_clear_color     = {0, 0, 0, 0};
            instance->m_resizable       = config.resizable;
        }
        return true;
    }

    Controller::Ptr Controller::get()
    {
        if(!instance)
            SP_ASSERT(create({}), "cannot create controller");
        return instance;
    }
    Controller::Controller() :
        m_listener{nullptr}
    {
        init();
    }

    Controller::~Controller()
    {
        delete m_listener;
        m_running = false;
        spExTerminate();
        if(static_cast<GLFWwindow*>(m_window) != NULL)
            glfwDestroyWindow(static_cast<GLFWwindow*>(m_window));
        glfwTerminate();
    }
    bool Controller::initController(const SP_Config& config)
    {
        glfwMakeContextCurrent(NULL);
        glfwTerminate();

        if(!glfwInit())
        {
            SP_PRINT_WARNING("failed to initialize backend API");
            return false;
        }
        GLFWwindow*         window;
        GLFWmonitor*        primary = glfwGetPrimaryMonitor();
        const GLFWvidmode*  mode    = glfwGetVideoMode(primary);

        //window properties..
        SP_PROPERTY(GLFW_RESIZABLE, config.resizable);
        SP_PROPERTY(GLFW_VISIBLE, config.visible);
        SP_PROPERTY(GLFW_DECORATED, config.decorated);
        SP_PROPERTY(GLFW_FOCUSED, config.focused);
        SP_PROPERTY(GLFW_AUTO_ICONIFY, config.iconify);
        SP_PROPERTY(GLFW_FLOATING, config.floating);
        SP_PROPERTY(GLFW_MAXIMIZED, config.maximized);
        SP_PROPERTY(GLFW_CENTER_CURSOR, config.centerCursor);
        SP_PROPERTY(GLFW_TRANSPARENT_FRAMEBUFFER, config.transparent);
        SP_PROPERTY(GLFW_FOCUS_ON_SHOW, config.focusOnShow);
        SP_PROPERTY(GLFW_SCALE_TO_MONITOR, config.scaleToMonitor);

        if(config.fullscreen)
        {
            SP_PROPERTY(GLFW_RED_BITS, mode->redBits);
            SP_PROPERTY(GLFW_GREEN_BITS, mode->greenBits);
            SP_PROPERTY(GLFW_BLUE_BITS, mode->blueBits);
            SP_PROPERTY(GLFW_REFRESH_RATE, mode->refreshRate);

            window = glfwCreateWindow(mode->width, mode->height, config.title, primary, NULL);
            glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            window = glfwCreateWindow(config.width, config.height, config.title, NULL, NULL);
        }

        if(!window)
        {
            SP_PRINT_WARNING("failed to create window");
            glfwTerminate();
            return false;
        }

        if(config.iconify)
            glfwIconifyWindow(window);

            glfwMakeContextCurrent(window);
        ///framebuffer properties..
        SP_PROPERTY(GLFW_RED_BITS, config.framebuffer.redBits);
        SP_PROPERTY(GLFW_GREEN_BITS, config.framebuffer.greenBits);
        SP_PROPERTY(GLFW_BLUE_BITS, config.framebuffer.blueBits);
        SP_PROPERTY(GLFW_ALPHA_BITS, config.framebuffer.alphaBits);
        SP_PROPERTY(GLFW_DEPTH_BITS, config.framebuffer.depthBits);
        SP_PROPERTY(GLFW_STENCIL_BITS, config.framebuffer.stencilBits);
        SP_PROPERTY(GLFW_ACCUM_RED_BITS, config.framebuffer.accumRedBits);
        SP_PROPERTY(GLFW_ACCUM_GREEN_BITS, config.framebuffer.accumGreenBits);
        SP_PROPERTY(GLFW_ACCUM_BLUE_BITS, config.framebuffer.accumBlueBits);
        SP_PROPERTY(GLFW_ACCUM_ALPHA_BITS, config.framebuffer.accumAlphaBits);
        SP_PROPERTY(GLFW_AUX_BUFFERS, config.framebuffer.auxBuffers);
        SP_PROPERTY(GLFW_STEREO, config.framebuffer.stereo);
        SP_PROPERTY(GLFW_SAMPLES, config.framebuffer.multisampling);
        SP_PROPERTY(GLFW_REFRESH_RATE, config.refreshRate);
        SP_PROPERTY(GLFW_STEREO, config.framebuffer.stereo);
        SP_PROPERTY(GLFW_SRGB_CAPABLE, config.framebuffer.saturateRGB);
        SP_PROPERTY(GLFW_DOUBLEBUFFER, config.doubleBuffer);


			///context properties..
        SP_PROPERTY(GLFW_CLIENT_API, static_cast<int>(config.client));
        SP_PROPERTY(GLFW_CONTEXT_CREATION_API, static_cast<int>(config.contextCreation));
        SP_PROPERTY(GLFW_CONTEXT_VERSION_MAJOR, config.contextMajor);
        SP_PROPERTY(GLFW_CONTEXT_VERSION_MINOR, config.contextMinor);
        SP_PROPERTY(GLFW_CONTEXT_ROBUSTNESS, static_cast<int>(config.robustness));
        SP_PROPERTY(GLFW_CONTEXT_RELEASE_BEHAVIOR, static_cast<int>(config.release));
        SP_PROPERTY(GLFW_OPENGL_FORWARD_COMPAT, config.forwardCompatible);
        SP_PROPERTY(GLFW_OPENGL_DEBUG_CONTEXT, config.debugContext);
        SP_PROPERTY(GLFW_OPENGL_PROFILE, static_cast<int>(config.profile));

        m_window 	= window;
        m_primary 	= static_cast<GLFWmonitor*>(primary);
        m_videoMode = static_cast<GLFWvidmode*>(const_cast<GLFWvidmode*>(mode));

        if(!spExInit())
        {
            SP_PRINT_WARNING("failed to initialize controller API");
            return false;
        }

        if(spGLLoadExtensions() != SPGL_OK)
        {
            SP_PRINT_WARNING("failed to load extensions");
        }
        srand(time(NULL));

        Controller::m_running = true;
        /*
        GLFWimage images[1];
        images[0].pixels = reinterpret_cast<unsigned char*>(const_cast<void*>(loadDefaultIcon()));
        images[0].width  = (int)64;
        images[0].height = (int)71;
        glfwSetWindowIcon(window, 1, images);
        */
        return true;
    }

    void Controller::setResizable(bool resize)
    {
        m_resizable = resize;
        SP_PROPERTY(GLFW_RESIZABLE, resize);
    }

    void Controller::terminate()
    {
        if(!m_window_active)
            return;

        GLFWwindow* window = static_cast<GLFWwindow*>(const_cast<void*>(Controller::getHandle()));
        glfwSetWindowShouldClose(window, true);

        m_window_active = false;
    }

    void Controller::init()
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(const_cast<void*>(Controller::getHandle()));

        //addEvent<SysI2Event>(SystemTag::WindowMove);
        glfwSetWindowUserPointer(window, this);
        glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xpos, int ypos){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowPos(xpos, ypos);
		});

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int xpos, int ypos){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowSize(xpos, ypos);
		});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowClose();
		});

		glfwSetWindowRefreshCallback(window, [](GLFWwindow* window){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowRefresh();
		});

		glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focus){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowFocus(focus);
		});

		glfwSetWindowIconifyCallback(window, [](GLFWwindow* window, int iconify){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowIconify(iconify);
		});

		glfwSetWindowMaximizeCallback(window, [](GLFWwindow* window, int maximize){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowMaximize(maximize);
		});

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowFramebufferSize(width, height);
		});

		glfwSetWindowContentScaleCallback(window, [](GLFWwindow* window, float width, float height){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnWindowContentScale(width, height);
		});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnKey(key, scancode, action, mods);
		});

		glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnChar(codepoint);
		});

		glfwSetCharModsCallback(window, [](GLFWwindow* window, unsigned int codepoint, int mods){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnCharMods(codepoint, mods);
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnMouseButton(button, action, mods);
		});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnCursorPos(xpos, ypos);
		});

		glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int enter){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnCursorEnter(enter);
		});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xpos, double ypos){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnScroll(xpos, ypos);
		});

		glfwSetDropCallback(window, [](GLFWwindow* window, int pathCount, const char* paths[]){
			static_cast<Controller*>(glfwGetWindowUserPointer(window))->fnDrop(pathCount, paths);
		});
    }

    const void* Controller::getHandle()
    {
        return m_window;
    }

    bool Controller::running() const
    {
        return m_window_active;
    }

    void Controller::dispatch(SP_Detail* detail)
    {
        auto it = m_subscriptions.find(detail->event);
        if(it != m_subscriptions.end())
            it->second.broadcast(detail);
    }

    void Controller::Controller::fnWindowPos(int xpos, int ypos)
    {
        getSubject()->dispatch<sp::SysWindowMove>(SysWindowMove{xpos, ypos});
        dispatch<SP_Detail>(SP_SystemEvent::WindowMove);
    }

    void Controller::Controller::fnWindowSize(int width, int height)
    {
        getSubject()->dispatch<sp::SysWindowResize>(SysWindowResize{width, height});
        m_size.x = width;
        m_size.y = height;
        m_renderer.setViewport(rectf{0, 0, (float)width, (float) height});
        m_renderer.setSurfaceSize(width, height);
        dispatch<SP_Detail>(SP_SystemEvent::WindowSize, width, height);
    }

    void Controller::fnWindowClose()
    {
        //m_window_active = false;
        getSubject()->dispatch<sp::SysWindowClose>(SysWindowClose{});
        dispatch<SP_Detail>(SP_SystemEvent::WindowClose);
        terminate();
    }

    void Controller::fnWindowRefresh()
    {
        getSubject()->dispatch<sp::SysWindowRefresh>(SysWindowRefresh{});
        dispatch<SP_Detail>(SP_SystemEvent::WindowRefresh);
    }

    void Controller::fnWindowFocus(int focused)
    {
        getSubject()->dispatch<sp::SysWindowFocus>(SysWindowFocus{focused});
        dispatch<SP_Detail>(SP_SystemEvent::WindowFocus, focused);
    }

    void Controller::fnWindowIconify(int iconified)
    {
        getSubject()->dispatch<sp::SysWindowIconify>(SysWindowIconify{iconified});
        dispatch<SP_Detail>(SP_SystemEvent::WindowIconify, iconified);
    }

    void Controller::fnWindowMaximize(int maximized)
    {
        getSubject()->dispatch(SysWindowMaximize{maximized});
        dispatch<SP_Detail>(SP_SystemEvent::WindowMaximize, maximized);
    }

    void Controller::fnWindowFramebufferSize(int width, int height)
    {
        getSubject()->dispatch(SysWindowFramebufferResize{width, height});
        dispatch<SP_Detail>(SP_SystemEvent::WindowFramebufferSize, width, height);
    }

    void Controller::fnWindowContentScale(int width, int height)
    {
        getSubject()->dispatch(SysWindowContentScale{width, height});
        dispatch<SP_Detail>(SP_SystemEvent::WindowContentScale, width, height);
    }

    /**< input events.. */
    void Controller::fnKey(int key, int scancode, int action, int mods)
    {
        getSubject()->dispatch(SysEventKey{key, scancode, action, mods});
        dispatch<SP_Detail>(SP_SystemEvent::EventKey, key, scancode, action, mods);
    }

    void Controller::fnChar(unsigned int codepoint)
    {
        getSubject()->dispatch(SysEventChar{codepoint});
        dispatch<SP_Detail>(SP_SystemEvent::EventChar, codepoint);
    }

    void Controller::fnCharMods(unsigned int codepoint, int mods)
    {
        getSubject()->dispatch(SysEventCharMods{codepoint, mods});
        dispatch<SP_Detail>(SP_SystemEvent::EventCharMods, codepoint, mods);
    }

    void Controller::fnMouseButton(int button, int action, int mods)
    {
        getSubject()->dispatch(SysEventMouseButton{button, action, mods});
        dispatch<SP_Detail>(SP_SystemEvent::EventMouseButton, button, action, mods);
    }

    void Controller::fnCursorPos(double xpos, double ypos)
    {
        getSubject()->dispatch(SysEventCursorPos{xpos, ypos});
        dispatch<SP_Detail>(SP_SystemEvent::EventCursorPos, xpos, ypos);
    }

    void Controller::fnCursorEnter(int entered)
    {
        getSubject()->dispatch(SysEventCursorEnter{entered});
        dispatch<SP_Detail>(SP_SystemEvent::EventCursorEnter, entered);
    }

    void Controller::fnScroll(double xoffset, double yoffset)
    {
        getSubject()->dispatch(SysEventScroll{xoffset, yoffset});
        dispatch<SP_Detail>(SP_SystemEvent::EventScroll, xoffset, yoffset);
    }

    void Controller::fnDrop(int pathCount, const char* paths[])
    {
        getSubject()->dispatch(SysEventDrop{pathCount, paths});
        dispatch<SP_Detail>(SP_SystemEvent::EventDrop, pathCount, paths);
    }

    /*
    void Controller::resetView(const sp::rectf& rect)
    {
        m_center.x = rect.left + rect.width  / 2.f;
        m_center.y = rect.top  + rect.height / 2.f;

        float width  = rect.width;
        float height = rect.height;

        float vpl = (int) .5f;
        float vpt = (int) .5f;
        float vpw = vpl + width;
        float vph = vpt + height;

        int top = height - (vpt + vph);
        glViewport(vpl, top, vpw, vph);

        float a =  2.f / width;
        float b = -2.f / height;
        float c = -a * m_center.x;
        float d = -b * m_center.y;

        matrix[0] = a;
        matrix[5] = b;

        matrix[12] = c;
        matrix[13] = d;

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(matrix);
        glMatrixMode(GL_MODELVIEW);
    }


    void Controller::beginDraw()
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);


        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushMatrix();
    }

    void Controller::endDraw()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glPopMatrix();
    }
    */

    /*
    void Controller::draw(const Drawable& draw, const States& states)
    {
        m_target.draw(draw, states);
    }
    */
    void Controller::setDrawCallback(std::function<void()> fn)
    {
        m_draw_callback = fn;
    }

    void Controller::setClearColor(const Color& color)
    {
        m_clear_color = color;
    }

    void Controller::mainLoop()
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
        if(!m_window)
        {
            SP_PRINT_WARNING("cannot use window");
            return;
        }
        /*
        m_listener = createObserver();
        m_listener->listen<SysWindowResize>([this](const SysWindowResize& event)
        {
            printf("window resized: %d %d\n", event.data.x, event.data.y);
        });

        m_listener->listen<SysWindowClose>([this](const SysWindowClose& event)
        {
            printf("closed window..\n");
        });
        */

        //m_listener.unlisten<SysWindowClose>();

        //glfwGetWindowSize(window, &m_size.x, &m_size.y);

        //resetView(sp::rectf{0, 0, (float)m_size.x, (float)m_size.y});

        /*
        m_target.initialize();
        m_target.setSurfaceSize(m_size.x, m_size.y);
        m_target.setViewport(rectf{0, 0, (float)m_size.x, (float) m_size.y});
        */

        m_renderer.initialize();
        m_renderer.setViewport(rectf{0, 0, (float)m_size.x, (float) m_size.y});
        m_renderer.setSurfaceSize(m_size.x, m_size.y);

        float deltaTime = 0.f;
        float lastFrame = 0.f;


        //printf("bus address: %")
        std::cout << "bus address: " << getSubject().get() << std::endl;
        std::chrono::steady_clock::time_point lastRenderTime;
        while(running())
        {
            const auto timePointNow = std::chrono::steady_clock::now();
            const auto timePointNextAllowed = lastRenderTime + std::chrono::milliseconds(20);
            if(timePointNextAllowed <= timePointNow)
            {
                //beginDraw();
                //m_target.clear(m_clear_color);
                //m_target.draw(sprite);

                m_renderer.clear(m_clear_color);
                m_renderer.draw();

                /*if(m_draw_callback)
                    m_draw_callback();

                */
                //endDraw();
                glfwSwapBuffers(window);
            }

            float currentFrame = (float) glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            double eventTimeoutSeconds = .0001d;

            sp::Optional<sp::Duration> duration = sp::Timer::getNextScheduledTime();
            if(duration && (*duration < std::chrono::milliseconds(10)))
                eventTimeoutSeconds = static_cast<double>(duration->asSeconds());

            glfwWaitEventsTimeout(eventTimeoutSeconds);
            getSubject()->process();

            //glfwPollEvents();
            getSubject()->dispatch<sp::SysEventUpdate>(SysEventUpdate{deltaTime});
            dispatch<SP_Detail>(SP_SystemEvent::EventUpdate, deltaTime);

            /*
            static int frames = 0;
            static StopWatch frameTimer;
            frames++;
            if(frames == 100)
            {
                float fps;
                char buffer[64];
                fps = 100.f / frameTimer.getElapsedSeconds();

                sprintf(buffer, "performance: %.1f fps", fps);
                glfwSetWindowTitle(window, buffer);
                frameTimer.reset();
                frames = 0;
            }
            */
        }
    }

    const Viewport& Controller::getViewport() const
    {
        return m_viewport;
    }

    /*
    void Controller::pushStates()
    {
        m_target.pushStatesGL();
    }
    void Controller::popStates()
    {
        m_target.popStatesGL();
    }
    */

    bool Controller::active(){return m_running;}

    void Controller::addDrawable(const Drawable::Ptr primitive, bool set_max)
    {
        m_renderer.addDrawable(primitive, set_max);
    }

    void Controller::removeDrawable(const Drawable::Ptr drawable)
    {
        m_renderer.removeDrawable(drawable);
    }

    void Controller::draw(const Drawable::Ptr primitive)
    {
        m_renderer.draw(primitive);
    }
    void Controller::setPostProcessShader(const sp::Shader* shader)
    {
        m_renderer.setPostProcessShader(shader);
    }

    void Controller::setFramebufferPosition(const vec2f& pos)
    {
        m_renderer.setFramebufferPosition(pos.x, pos.y);
    }
    void Controller::setFramebufferPosition(float x, float y)
    {
        m_renderer.setFramebufferPosition(x, y);
    }
    unsigned int Controller::getFramebufferTexHandleGL() const
    {
        return m_renderer.getFramebufferTexHandleGL();
    }

    void Controller::loadImageIcon(const char* path)
    {

        GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
        if(!window)
            return;

        unsigned int width, height;
        unsigned char* data = spHelperFileImage(path, &width, &height, NULL, 4);
        GLFWimage images[1];
        images[0].pixels = data;
        images[0].width  = (int)width;
        images[0].height = (int)height;

        glfwSetWindowIcon(window, 1, images);
        free(data);
    }

    void Controller::setImageIcon(unsigned char* pixels)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
        if(!window)
            return;

        GLFWimage images[1];
        images[0].pixels = pixels;

        glfwSetWindowIcon(window, 1, images);
    }

    vec2f Controller::mapPixelsToCoords(const vec2i& point)
    {
        return m_renderer.mapPixelsToCoords(point.x, point.y);
    }
    vec2i Controller::mapCoordsToPixels(const vec2f& point)
    {
        return m_renderer.mapCoordsToPixels(point.x, point.y);
    }

    Observer::Ptr Controller::createObserver()
    {
        return std::make_shared<Observer>(getSubject());
    }

    Machine::Ptr Controller::getContainer()
    {
        return s_container;
    }

    Machine::ConstPtr Controller::getContainer() const
    {
        return s_container;
    }
}
