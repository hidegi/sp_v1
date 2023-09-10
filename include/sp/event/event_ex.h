#ifndef EVENT_EX_H_INCLUDED
#define EVENT_EX_H_INCLUDED
#include <sp/exsp/event_bus.h>

namespace sp
{
    enum class Flag : int
	{
	    NoOperation,
	    PostponeCancel,
	    PostponeContinue
	};

	enum class SP_API SystemTag : long int
	{
			None	= 0,
			WindowMove,
			WindowSize,
			WindowClose,
			WindowRefresh,
			WindowFocus,
			WindowIconify,
			WindowMaximize,
			WindowFramebufferSize,
			WindowContentScale,
			EventMouseButton,
			EventCursorPos,
			EventCursorEnter,
			EventScroll,
			EventKey,
			EventChar,
			EventCharMods,
			EventDrop,
			EventUpdate,
			UserEvent,
			SystemEventCount = EventUpdate + 1 //19..
	};

	template<typename E, SystemTag>
	class SP_API SystemData
	{
        using Event = E;

        public:
            typedef E type;
            Event       data;
	};

	struct SP_API SysI2Data
	{
        int x;
        int y;
	};

	struct SP_API SysD2Data
	{
	    double x;
        double y;
	};

	struct SP_API SysKeyData
	{
        int key;
        int scancode;
        int action;
        int mods;
	};

	struct SP_API SysCharModsData
	{
	    unsigned int charmods;
                 int mods;
	};

    struct SP_API SysDropData
    {
        int pathCount;
        const char** paths;
    };

    struct SP_API SysButtonData
    {
        int button;
        int action;
        int mods;
    };

    struct SP_API SysVoidData
    {
    };

    struct SP_API SysUintData
    {
        unsigned int data;
    };

    struct SP_API SysIntData
    {
        int data;
    };

    struct SP_API SysDoubleData
    {
        double data;
    };

	template<typename T>
	struct SP_API Sys
	{
	    using Event = T;
        SystemTag   tag;
        Event       data;
	};


	using SysWindowMove                 = SystemData<SysI2Data, SystemTag::WindowMove>;
	using SysWindowResize               = SystemData<SysI2Data, SystemTag::WindowSize>;
	using SysWindowClose                = SystemData<SysVoidData, SystemTag::WindowClose>;
	using SysWindowRefresh              = SystemData<SysVoidData, SystemTag::WindowRefresh>;
	using SysWindowFocus                = SystemData<SysIntData, SystemTag::WindowFocus>;
	using SysWindowIconify              = SystemData<SysIntData, SystemTag::WindowIconify>;
	using SysWindowMaximize             = SystemData<SysIntData, SystemTag::WindowMaximize>;
	using SysWindowFramebufferResize    = SystemData<SysI2Data, SystemTag::WindowFramebufferSize>;
	using SysWindowContentScale         = SystemData<SysI2Data, SystemTag::WindowContentScale>;
	using SysEventKey                   = SystemData<SysKeyData, SystemTag::EventKey>;
	using SysEventChar                  = SystemData<SysUintData, SystemTag::EventChar>;
	using SysEventCharMods              = SystemData<SysCharModsData, SystemTag::EventCharMods>;
	using SysEventMouseButton           = SystemData<SysButtonData, SystemTag::EventMouseButton>;
	using SysEventCursorPos             = SystemData<SysD2Data, SystemTag::EventCursorPos>;
	using SysEventCursorEnter           = SystemData<SysIntData, SystemTag::EventCursorEnter>;
	using SysEventScroll                = SystemData<SysD2Data, SystemTag::EventScroll>;
	using SysEventDrop                  = SystemData<SysDropData, SystemTag::EventDrop>;
	using SysEventUpdate                = SystemData<SysDoubleData, SystemTag::EventUpdate>;
    /*
    struct SysWindowMove
	{
	    SysI2Data data;
	};

	struct SysWindowResize
	{
	    SysI2Data data;
	};
	struct SysWindowClose
	{
	};
	struct SysWindowRefresh
	{
	};
	struct SysWindowFocus
	{
	    SysIntData data;
	};
	struct SysWindowIconify
	{
	    SysIntData data;
	};
	struct SysWindowMaximize
	{
	    SysIntData data;
	};
	struct SysWindowFramebufferResize
	{
	    SysI2Data data;
	};
	struct SysWindowContentScale
	{
	    SysI2Data data;
	};
	struct SysEventKey
	{
	    SysKeyData data;
	};
	struct SysEventChar
	{
	    SysUintData data;
	};
	struct SysEventCharMods
	{
	    SysCharModsData data;
	};
	struct SysEventMouseButton
	{
	    SysButtonData data;
	};
	struct SysEventCursorPos
	{
	    SysD2Data data;
	};
	struct SysEventCursorEnter
	{
	    SysIntData data;
	};
	struct SysEventScroll
	{
	    SysD2Data data;
	};
	struct SysEventDrop
	{
	    SysDropData data;
	};
	struct SysEventUpdate
	{
	    SysDoubleData data;
	};
	*/
	class AbstractEvent
	{
        public:
            virtual ~AbstractEvent() {}
	};

	class SP_API SystemBus : public sp::EventBus
	{
        public:
            class Delegate
            {
                friend class SystemBus;
                public:

                    //_listener.listen<Gold>(std::bind(&ShopButton::onGoldUpdated, this, std::placeholders::_1));
                    template<typename E>
                    Delegate& setInvokeImmediate(Flag(E::* fn)(sp::bus::PostponeHelper&))
                    {
                        _bus->_invoke_direct.push_back(std::bind(fn, static_cast<E*>(_event), std::placeholders::_1));
                        return *this;
                    }

                    template<typename E>
                    Delegate& setInvokeLater(Flag(E::* fn)(sp::bus::PostponeHelper&))
                    {
                        _bus->_invoke_later.push_back(std::bind(fn, static_cast<E*>(_event), std::placeholders::_1));
                        return *this;
                    }
                private:
                    SystemBus*          _bus;
                    AbstractEvent*      _event;

                    Delegate(SystemBus* bus, AbstractEvent* event) :
                        _bus    {bus},
                        _event  {event}
                    {
                    }
            };

            Delegate addEvent(std::shared_ptr<AbstractEvent> event)
            {
                auto local = event.get();
                _events.push_back(std::move(event));
                return Delegate(this, local);
            }

        protected:
            bool postponeEvent(sp::bus::PostponeHelper& postponeCall) override
            {
                for(const auto& pre_postpone : _invoke_direct)
                {
                    if(pre_postpone(postponeCall) == Flag::PostponeCancel)
                        return false;
                }

                if(EventBus::postponeEvent(postponeCall))
                {
                    for(const auto& post_postpone : _invoke_later)
                    {
                        if(post_postpone(postponeCall) == Flag::PostponeCancel)
                            break;
                    }
                    return true;
                }
                return false;
            }

        private:
            template<typename E>
            E* getEvent()
            {
                auto found = std::find_if(_events.begin(), _events.end(), [](const std::shared_ptr<AbstractEvent>& event)
                {
                    return dynamic_cast<E*>(event) != nullptr;
                });

                if(found != _events.end())
                {
                    return static_cast<E*>(found->get());
                }
                return nullptr;
            }

        private:
            std::vector<std::shared_ptr<AbstractEvent>>                     _events;
            std::vector<std::function<Flag(sp::bus::PostponeHelper&)>>      _invoke_direct;
            std::vector<std::function<Flag(sp::bus::PostponeHelper&)>>      _invoke_later;
	};

	//bus->postpone(SystemEvent<WindowMove>(10, 10));   //ok..
	//bus->postpone(SystemEvent<WindowResize>(10, 10)); //ok..
	//bus->postpone(SystemEvent<WindowClose>(10, 10));  //no!!
	//bus->postpone(SystemEvent<WindowClose>());  //ok..
	//bus->postpone(SystemEvent<WindowMaximize>(1));  //ok..



	struct SP_API SystemEvent : public AbstractEvent
	{
        public:
            SystemEvent(SystemTag tag, sp::bus::Bus* owner) :
                _system_tag {tag},
                _owner      {owner}
            {
            }

            Flag onImmediateInvokeEvent(sp::bus::PostponeHelper& postponeCall)
            {
                auto found = _event_callbacks.find(postponeCall.eventID);
                if(found != _event_callbacks.end())
                {
                    found->second(postponeCall.event);
                    return Flag::PostponeCancel;
                }
                return Flag::PostponeContinue;
            }

            template<typename Sys>
            SystemEvent& create()
            {
                static_assert(sp::inter::is_event_valid<Sys>(), "invalid system event..");
                static_assert(sp::inter::is_event_valid<typename Sys::Event>(), "invalid system event..");
                SP_CONSTEXPR auto eventID = sp::inter::type_id<typename Sys::Event>();
                _event_callbacks[eventID] = SP_LAMBDA_CAPTURE_EQ_THIS(sp::Any event)
                {
                    Sys newEvent{_system_tag, std::move(sp::any_cast<typename Sys::Event>(event))};
                    _owner->postpone<Sys>(std::move(newEvent));
                };
                return *this;
            }

        private:
            std::map<sp::inter::type_id_t, std::function<void(sp::Any)>>   _event_callbacks;
            SystemTag                                                       _system_tag;
            sp::bus::Bus*                                                   _owner;
	};

	template<typename S>
	static void SP_API addEvent(SystemTag tag)
	{
        auto bus = std::make_shared<SystemBus>();
        {
            auto tag_event = std::make_unique<SystemEvent>(tag, bus.get());
            tag_event->create<Sys<S>>();
            bus->addEvent(std::move(tag_event));
        }
	}

}

#endif // EVENT_EX_H_INCLUDED
