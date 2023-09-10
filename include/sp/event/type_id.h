#ifndef TYPE_ID_H_INCLUDED
#define TYPE_ID_H_INCLUDED
#include <sp/sp.h>
namespace sp
{
    //every class template instantiation will have a different
    //location in memory for static variable id..

    using type_id_t = const uint32_t;

    namespace detail
    {
        type_id_t hash(const char* str);
    }


    export template<typename T>
    static SP_CONSTEXPR type_id_t type_id()
    {
        //return reinterpret_cast<const uintptr_t>(&type_id_ptr<T>::id);
        const type_id_t id = detail::hash(typeid(T).name());
        return id;
    }

    /*
    export template<template <typename, typename ... > typename W, typename E, typename ... Args>
    static SP_CONSTEXPR type_id_t type_id()
    {
        using type = W<E, Args...>;
        const type_id_t id = detail::hash(typeid(E).name());
        return id;
    }
    */


    /*

    SP_CONSTEXPR type_id_t type_id()
    {
        const type_id_t id = detail::hash(typeid(T<E>).name());
        return id;
    }
    */


    export template<class E>
    SP_CONSTEXPR bool is_event_valid()
    {
        static_assert(!std::is_const<E>::value, "cannot have const-qualifier for event..");
        static_assert(!std::is_volatile<E>::value, "cannot have volatile-qualifier for event..");
        static_assert(!std::is_reference<E>::value, "cannot have event as reference..");
        static_assert(!std::is_pointer<E>::value, "cannot have event as pointer..");
        return true;
    }
}



#endif // TYPE_ID_H_INCLUDED
