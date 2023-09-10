#ifndef LISTENER_ATTORNEY_H_INCLUDED
#define LISTENER_ATTORNEY_H_INCLUDED
#include <sp/exsp/type_id.h>
#include <functional>

namespace sp
{
    namespace bus
    {
        template<typename>
        class ListenerSPRB;
    }
    namespace inter
    {
        template<typename EventBus_t>
        class ListenerAttorney
        {
            private:
                template<typename>
                friend class sp::bus::ListenerSPRB;
            public:
                static SP_CONSTEXPR uint32_t newListenerID(EventBus_t& bus)
                {
                    return bus.newListenerID();
                }

                template<class Event>
                static SP_CONSTEXPR void listen(EventBus_t& bus, const uint32_t listenerID,
                                                std::function<void(const Event&)>&& callback)
                {
                    bus.template listen<Event>(listenerID, std::forward<std::function<void(const Event&)>>(callback));
                }

                static SP_CONSTEXPR void unlistenAll(EventBus_t& bus, const uint32_t listenerID)
                {
                    bus.unlistenAll(listenerID);
                }

                static SP_CONSTEXPR void unlisten(EventBus_t& bus, const uint32_t listenerID,
                                                  const type_id_t eventID)
                {
                    bus.unlisten(listenerID, eventID);
                }
        };
    }
}


#endif // LISTENER_ATTORNEY_H_INCLUDED
