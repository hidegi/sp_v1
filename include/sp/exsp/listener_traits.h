#ifndef LISTENER_TRAITS_H_INCLUDED
#define LISTENER_TRAITS_H_INCLUDED
#include <functional>
#include <type_traits>

namespace sp::inter
{
    //template<float, int, Obj1, Obj2>
    /*
     *  float sp(int, Obj1, Obj2) { ... }
     *  float (sp_ptr*)(int, Obj1, Obj2);
     */
    //int first_argument_helper(sp_ptr);
    //int first_argument_helper(float ( fn_ptr*)(int, Obj1, Obj2));

    template<typename R, typename Arg, typename ... Args>
    Arg first_argument_helper(R(*)(Arg, Args...));

    template<typename R, typename F, typename Arg, typename ... Args>
    Arg first_argument_helper(R(F::*)(Arg, Args...));

    template<typename R, typename F, typename Arg, typename ... Args>
    Arg first_argument_helper(R(F::*)(Arg, Args...) const);

    template<typename F>
    decltype(first_argument_helper(&F::operator())) first_argument_helper(F);

    //declval is commonly used in templates, where acceptable template parameters
    //may have no common constructor, though a common member function, whose return
    //type is needed..
    template<typename T>
    using first_argument = decltype(first_argument_helper(std::declval<T>()));
}


#endif // LISTENER_TRAITS_H_INCLUDED
