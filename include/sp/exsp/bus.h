#ifndef BUS_H_INCLUDED
#define BUS_H_INCLUDED
#include <any>
#include <atomic>
#include <memory>

#include <sp/exsp/listener_sprb.h>
#include <sp/exsp/listener_attorney.h>
#include <sp/exsp/type_id.h>
#include <sp/exsp/protected_event_stream.h>

//the event xsp software interface..
//using an observer pattern that acts upon events..

namespace sp::bus
{
    class Bus;
    template<typename Event>
    using DefaultEventStream = bus::stream::ProtectedEventStream<Event>;
    using CreateStreamCallback = std::unique_ptr<bus::stream::EventStream>(*const)();
    using PostPoneCallback = bool(*const)(Bus& bus, sp::Any event);

    template<typename Event>
    bool postpone(Bus& bus, sp::Any event);

    template<typename Event>
    std::unique_ptr<bus::stream::EventStream> createDefaultEventStream()
    {
        return std::make_unique<DefaultEventStream<Event>>();
    }

    //helper class for storing event states..
    class SP_API PostponeHelper
    {
        public:
            inter::type_id_t eventID = nullptr;

            //cannot have cv-qualifier, neither be ref-nor pointer..
            sp::Any event;

            PostPoneCallback postponeCallback = nullptr;
            CreateStreamCallback createStreamCallback = nullptr;

            PostponeHelper(const inter::type_id_t _eventID, sp::Any&& _event,
                           PostPoneCallback postponeCallback,
                           CreateStreamCallback createStreamCallback) :
                eventID(_eventID),
                event(std::forward<sp::Any>(_event)),
                postponeCallback(postponeCallback),
                createStreamCallback(createStreamCallback)
            {
            }

            //factory function..
            template<typename Event>
            static PostponeHelper create(sp::Any&& event)
            {
                return PostponeHelper(inter::type_id<Event>(),
                                      std::forward<sp::Any>(event),
                                      postpone<Event>,
                                      createDefaultEventStream<Event>);
            }
            ~PostponeHelper() = default;
    };

    class SP_API Bus
    {
        template<typename>
        friend class sp::inter::ListenerAttorney;

        public:
            using ListenerSPRB = bus::ListenerSPRB<sp::bus::Bus>;
            Bus() = default;
            virtual ~Bus() = default;

            virtual size_t process() = 0;
            template<typename Event>
            bool postpone(Event event)
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                auto postponeCall = PostponeHelper::create<Event>(std::move(event));
                return postponeEvent(postponeCall);
            }

        protected:
            virtual bool postponeEvent(PostponeHelper& postponeCall) = 0;
            virtual bus::stream::EventStream* listen(uint32_t listenerID, inter::type_id_t eventID, CreateStreamCallback cb) = 0;
            virtual void unlistenAll(uint32_t listenerID) = 0;
            virtual void unlisten(uint32_t listenerID, inter::type_id_t eventID) = 0;

        private:
            std::atomic<uint32_t> _lastID{0};
            //uint32_t _lastID = 0;
            uint32_t newListenerID()
            {
                return ++_lastID;
            }

            template<class Event>
            void listen(const std::uint32_t listenerID, std::function<void(const Event&)>&& callback)
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                assert(callback != nullptr);
                SP_CONSTEXPR auto eventID = inter::type_id<Event>();
                bus::stream::EventStream* eventStream = listen(listenerID, eventID, createDefaultEventStream<Event>);

                if(eventStream != nullptr)
                {
                    printf("bus listening invoked..\n");
                    eventStream->addListener(listenerID, std::forward<std::function<void(const Event&)>>(callback));
                }
                else
                {
                    SP_PRINT_WARNING("event stream = nullptr");
                }
            }
    };

    template<typename Event>
    bool SP_API postpone(Bus& bus, sp::Any event)
    {
        return bus.postpone(std::move(any_cast<Event>(event)));
    }
}
#endif // BUS_H_INCLUDED
