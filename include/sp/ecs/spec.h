#ifndef SPEC_H_INCLUDED
#define SPEC_H_INCLUDED
#include <sp/sp.h>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace sp
{
    template<typename T>
    class Wrapper
    {
        public:
            using type = T;
            SP_CONSTEXPR Wrapper(size_t index) :
                index{index}
            {
            }

            const size_t index;
    };

    template<typename ... Ts>
    struct Identifier : public Wrapper<Ts>...
    {
        template<size_t ... Indices>
        SP_CONSTEXPR Identifier(std::index_sequence<Indices...>) : Wrapper<Ts>{Indices}...
        {
        }

        template<typename T>
        SP_CONSTEXPR size_t get() const {return Wrapper<typename std::decay<T>::type>::index;}
    };

    template<typename ... Ts>
    SP_CONSTEXPR auto ident = Identifier<typename std::decay<Ts>::type...>{std::make_index_sequence<sizeof...(Ts)>{}};
}



#endif // SPEC_H_INCLUDED
