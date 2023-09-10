#ifndef OBSERVER_ROOK_H_INCLUDED
#define OBSERVER_ROOK_H_INCLUDED
#include <sp/sp.h>
#include <sp/event/type_id.h>

namespace sp
{
    struct SP_API ObserverRook
    {
        template<typename E>
        static void connect(std::shared_ptr<Subject> sub, const uint32_t name, Callback<E>&& callback)
        {
            sub->registerObserver<E>(name, std::forward<Callback<E>>(callback));
        }

        template<typename E>
        static void disconnect(std::shared_ptr<Subject> sub, const uint32_t name)
        {
            sub->removeObserver<E>(name);
        }

        static void disconnectAll(std::shared_ptr<Subject> sub, const uint32_t name)
        {
            sub->removeObserverAnywhere(name);
        }
    };
}

#endif // OBSERVER_ROOK_H_INCLUDED
