#ifndef SP_TYPE_ID_H_INCLUDED
#define SP_TYPE_ID_H_INCLUDED
#include <sp/sp.h>
#include <type_traits>

//creates one unique id for an event (not instance)..
namespace sp::inter
{
    template<typename T>
    struct type_id_ptr
    {
        static const T* const id;
    };

    template<typename T>
    const T* const type_id_ptr<T>::id = nullptr;

    using type_id_t = const void*;

    //returns a unique id for class T..
    template<typename T>
    SP_CONSTEXPR type_id_t type_id()
    {
        return &type_id_ptr<T>::id;
    }

    //events can neither have cv-qualifiers, nor be a reference or pointer..
    template<class E>
    SP_CONSTEXPR bool is_event_valid()
    {
        static_assert(std::is_const<E>::value == false, "cannot have const-qualified event..");
        static_assert(std::is_volatile<E>::value == false, "cannot have volatile-qualified event..");
        static_assert(std::is_reference<E>::value == false, "cannot have event as reference..");
        static_assert(std::is_pointer<E>::value == false, "cannot have event as pointer..");
        return true;
    }
}


#endif // EVENT_ID_H_INCLUDED
