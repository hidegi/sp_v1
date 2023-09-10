#ifndef LISTENER_SPRB_H_INCLUDED
#define LISTENER_SPRB_H_INCLUDED
#include <sp/sp.h>
#include <sp/exsp/listener_attorney.h>
#include <sp/exsp/listener_traits.h>
#include <sp/exsp/type_id.h>
#include <sp/exception.h>
#include <type_traits>
#include <memory>
namespace sp
{
    namespace
    {
        uint32_t lastID = 0;
    }
    namespace bus
    {
        class Bus;
    }
    template<typename T>
    SP_CONSTEXPR bool is_const_v = std::is_const<T>::value;

    template<typename T>
    SP_CONSTEXPR bool is_reference_v = std::is_const<T>::value;

    template<class Bus>
    class ListenerSPRB
    {
        private:
            std::shared_ptr<Bus>    _bus = nullptr;
            uint32_t                _id = 0;
        public:

            explicit ListenerSPRB() = default;
            explicit ListenerSPRB(std::shared_ptr<Bus> bus)
            {
                _bus = std::move(bus);
                _id  = ++lastID;//inter::ListenerAttorney<Bus>::newListenerID(*bus);
                if(!_bus)
                {
                    SP_PRINT_WARNING("bus = nullptr");
                }
            }

            static ListenerSPRB createNonOwning(Bus& bus)
            {
                return ListenerSPRB{std::shared_ptr<Bus>(&bus, [](Bus*){})};
            }

            ListenerSPRB(const ListenerSPRB&) = delete;
            ListenerSPRB(ListenerSPRB&&) = delete;
            ListenerSPRB& operator=(const ListenerSPRB&) = delete;
            ListenerSPRB& operator=(ListenerSPRB&&) = delete;

            ~ListenerSPRB()
            {
                if(_bus)
                {
                    unlistenAll();
                }
                --lastID;
            }

            void unlistenAll()
            {
                if(!_bus)
                {
                    throw SP_EXCEPTION("bus cannot be null");
                }
                inter::ListenerAttorney<Bus>::unlistenAll(*_bus, _id);
            }

            template<class Event, typename _ = void>
            SP_CONSTEXPR void listen(std::function<void(const Event&)>&& callback)
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                listenToCallback<Event>(std::forward<std::function<void(const Event&)>>(callback));
            }

            template<class EventCallback, typename Event = inter::first_argument<EventCallback>>
            SP_CONSTEXPR void listen(EventCallback&& callback)
            {
                static_assert(is_const_v<std::remove_reference<Event>>, "event must be const..") ;
                static_assert(is_reference_v<std::remove_reference<Event>>, "event must be const-reference..") ;
                using PureEvent = std::remove_const_t<std::remove_reference_t<Event>>;
                static_assert(inter::is_event_valid<PureEvent>(), "invalid event..");
                listenToCallback<PureEvent>(std::forward<EventCallback>(callback));
            }

            //both L-R-val listenToCallback definitions forward callback as a mere copy..
            //L-val..
            template<class Event>
            void listenToCallback(const std::function<void(const Event&)>& callback)
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                if(!_bus)
                {
                    throw SP_EXCEPTION("bus cannot be null");
                }


                inter::ListenerAttorney<Bus>::template listen<Event>(
                    *_bus,
                    _id,
                    std::forward<std::function<void(const Event&)>>(callback));
            }

            //R-val..
            template<class Event>
            void listenToCallback(std::function<void(const Event&)>&& callback)
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                if(!_bus)
                {
                    throw SP_EXCEPTION("bus cannot be null");
                }

                inter::ListenerAttorney<Bus>::template listen<Event>(
                    *_bus,
                    _id,
                    std::forward<std::function<void(const Event&)>>(callback));
            }

            template<typename Event>
            void unlisten()
            {
                static_assert(inter::is_event_valid<Event>(), "invalid event..");
                if(!_bus)
                {
                    throw SP_EXCEPTION("bus cannot be null");
                }

                printf("unlisten invoked..\n");

                inter::ListenerAttorney<Bus>::unlisten(*_bus, _id, inter::type_id<Event>());
            }

            void transfer(ListenerSPRB&& from)
            {
                if(this == &from)
                {
                    return;
                }

                if(!_bus)
                {
                    throw SP_EXCEPTION("bus cannot be null");
                }

                _id = from._id;
                _bus = std::move(from._bus);

            }

            const std::shared_ptr<Bus> get() const
            {
                return _bus;
            }
    };

    using BusListener = sp::ListenerSPRB<sp::bus::Bus>;
}


#endif // LISTENER_SPRB_H_INCLUDED
