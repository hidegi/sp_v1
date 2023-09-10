#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED
#include <vector>
#include <bitset>
#include <utility>
#include <cstddef>
#include <iterator>
#include <cassert>
#include <deque>
#include <stack>
#include <type_traits>
#include <sp/ecs/spec.h>
#include <sp/ecs/component_pool.h>

///welcome to STL-hell, yours faithful and beloved motex gaming 1667.. :D
namespace sp
{
    //a template parameter pack is a template parameter that accepts none,
    //one or more template arguments (non-types, types or even other templates)..
    //a template with at least one parameter pack is called a variadic template..
    template<typename ...>
    class Arrow;

    template<template<typename ... Args> class Pool, typename E, typename ... Cs, typename T, typename ... Ts>
    class Arrow<Pool<E, Cs...>, T, Ts...> final
    {
        using pool_type         = Pool<E, Cs...>;
        using entity_type       = typename pool_type::entity_type;

        //sizeof...(x) returns the number of elements in a parameter pack,
        //not the total byte count..
        using mask_type             = std::bitset<sizeof...(Cs) + 1>;
        using true_iterator_type    = typename pool_type::const_iterator_type;
        class ArrowIterator;

        public:
        using iterator_type         = ArrowIterator;
        using const_iterator_type   = iterator_type;
        using size_type             = typename pool_type::size_type;

        private:
            class ArrowIterator
            {
                private:
                    inline bool valid() const noexcept
                    {
                        return ((mask[*begin] & bitmask) == bitmask);
                    }

                public:
                using value_type        = entity_type;
                using difference_type   = std::ptrdiff_t;
                using reference         = entity_type&;
                using pointer           = entity_type*;

                /*
                 *  HELPER!!
                 *
                 *  vector:     random-access..
                 *  list:       bidirectional..
                 *  deque:      random-access..
                 *  map:        bidirectional..
                 *  multi-map:  bidirectional..
                 *  set:        bidirectional..
                 *  multi-set:  bidirectional..
                 *  stack:      none..
                 *  queue:      none..
                 *  priority-q: none..
                 */

                 /*
                  * random access
                  *       |
                  *       |------ bidirectional
                  *                     |
                  *                     |------ forward
                  *                                |
                  *                      |--------------------|
                  *                    input               output
                  *
                  * input:          read only, move forward..
                  * output:         write only, move forward..
                  * forward:        read/write, move forward..
                  * bidirectional:  read/write, move forward/backward..
                  * random access:  read/write, arbitrary access..
                  */

                  //read only iterator..
                using iterator_category = std::input_iterator_tag;

                ArrowIterator(true_iterator_type begin, true_iterator_type end,
                              const mask_type& bitmask, const mask_type* mask) noexcept :
                    begin   {begin},
                    end     {end},
                    bitmask {bitmask},
                    mask    {mask}
                {
                    if((begin != end) && !valid())
                            ++(*this);
                }

                ArrowIterator& operator++() noexcept
                {
                    ++begin;
                    while((begin != end) && !valid()) ++begin;
                    return *this;
                }

                ArrowIterator& operator++(int) noexcept
                {
                    ArrowIterator orig = *this;
                    //++(*this) is evaluated, then discarded, where orig is actually returned..
                    return ++(*this), orig;
                }

                bool operator==(const ArrowIterator& other) const noexcept
                {
                    return other.begin == begin;
                }

                bool operator!=(const ArrowIterator& other) const noexcept
                {
                    return !(*this == other);
                }

                value_type operator*() const noexcept
                {
                    return *begin;
                }

                private:
                    true_iterator_type      begin;
                    true_iterator_type      end;
                    const mask_type         bitmask;
                    const mask_type*        mask;
            };

            template<typename Comp>
            void prefer(size_type & size) noexcept
            {
                auto sz = pool.template size<Comp>();
                if(sz < size)
                {
                    from = pool.template begin<T>();
                    to   = pool.template end<T>();
                    size = sz;
                }
            }

        public:
            explicit Arrow(pool_type& pool, const mask_type* mask) noexcept :
                from    {pool.template begin<T>()},
                to      {pool.template end<T>()},
                pool    {pool},
                mask    {mask}
            {
                using accumulator_type = int[];
                size_type size = pool.template size<T>();
                bitmask.set(ident<Cs...>.template get<T>());
                accumulator_type types = {0, (bitmask.set(ident<Cs...>.template get<Ts>()), 0)  ...};
                accumulator_type pref  = {0, (prefer<Ts>(size), 0)...};
                (void) types, (void) pref;
            }

            const_iterator_type begin() const noexcept
            {
                return ArrowIterator{from, to, bitmask, mask};
            }

            iterator_type begin() noexcept
            {
                return const_cast<const Arrow*>(this)->begin();
            }

            const_iterator_type end() const noexcept
            {
                return ArrowIterator{to, to, bitmask, mask};
            }

            iterator_type end() noexcept
            {
                return const_cast<const Arrow*>(this)->end();
            }

            void reset() noexcept
            {
                using accumulator_type = int[];
                from = pool.template begin<T>();
                to   = pool.template end<T>();
                size_type size = pool.template size<T>();
                accumulator_type accumulator = {0, (prefer<Ts>(size), 0)...};

                //fake usage, so pedantic compiler won't issue warnings..
                (void)accumulator;
            }

        private:
            true_iterator_type      from;
            true_iterator_type      to;
            pool_type&              pool;
            const mask_type*        mask;
            mask_type               bitmask;
    };

    template<template<typename ... > class Pool, typename E, typename ... Cs, typename T>
    class Arrow<Pool<E, Cs...>, T> final
    {
        using pool_type = Pool<E, Cs...>;
        public:
            using size_type             = typename pool_type::size_type;
            using const_iterator_type   = typename pool_type::const_iterator_type;
            using iterator_type         = const_iterator_type;

            explicit Arrow(pool_type& pool) noexcept :
                pool{pool}
            {
            }

            const_iterator_type cbegin() const noexcept
            {
                //.template because cbegin itself is a template function..
                return pool.template cbegin<T>();
            }

            iterator_type begin() noexcept
            {
                return pool.template begin<T>();
            }

            const_iterator_type cend() const noexcept
            {
                return pool.template cend<T>();
            }

            iterator_type end() noexcept
            {
                return pool.template end<T>();
            }

            size_type size() const noexcept
            {
                return pool.template size<T>();
            }

        private:
            pool_type&                  pool;
    };

    template<typename>
    class Registry;

    template<template<typename ... > class Pool, typename E, typename ... Cs>
    class Registry<Pool<E, Cs...>>
    {
        static_assert(sizeof...(Cs) > 1, "cannot have no arguments..");
        using pool_type     = Pool<E, Cs...>;
        using mask_type     = std::bitset<sizeof...(Cs) + 1>;

        using IdentThis     = Identifier<Cs...>;
        static constexpr auto validity_bit = sizeof...(Cs);
        static constexpr IdentThis varlist = ident<Cs...>();

        public:
        using entity_type   = typename pool_type::entity_type;
        using size_type     = typename std::vector<mask_type>::size_type;

        private:
            template<typename Comp>
            void clone(entity_type to, entity_type from)
            {
                if(entities[from].test(ident<Cs...>.template get<Comp>()))
                    assign<Comp>(to, pool.template get<Comp>());
            }

            template<typename Comp>
            void sync(entity_type to, entity_type from)
            {
                bool src    = entities[from].test(ident<Cs...>.template get<Comp>());
                bool dst    = entities[to].test(ident<Cs...>.template get<Comp>());

                if(src && dst)
                    copy<Comp>(to, from);
                else if(src)
                    clone<Comp>(to, from);
                else if(dst)
                    remove<Comp>(to, from);
            }

        public:
            template<typename ... Comp>
            using arrow_type = Arrow<pool_type, Comp...>;

            template<typename ... Args>
            Registry(Args&& ... args) :
                pool{std::forward<Args>(args)...}
            {
            }

            Registry(Registry const&) = delete;
            Registry(Registry&&) = delete;
            Registry& operator=(Registry const&) = delete;
            Registry& operator=(Registry&&) = delete;

            size_type size() const noexcept
            {
                return entities.size() - available.size();
            }

            size_type capacity() const noexcept
            {
                return entities.size();
            }

            template<typename Comp>
            bool empty() const noexcept
            {
                return pool.template empty();
            }

            bool empty() const noexcept
            {
                return entities.empty();
            }

            bool valid(entity_type entity) const noexcept
            {
                return (entity < entities.size() && entities[entity].test(validity_bit));
            }

            template<typename ... Comp>
            entity_type create() noexcept
            {
                using accumulator_type = int[];
                auto entity = create();
                accumulator_type accumulator = {0, (assign<Comp>(entity), 0)...};
                (void) accumulator;
                return entity;
            }

            ///entry point..
            //returns the last size of entities, if available is empty..
            //subtle id generation; also helps for validation..
            entity_type create() noexcept
            {
                entity_type entity;

                //an entity's id is put into a cache stack that has been
                //previously deleted..
                //therefore, any deleted ids will be returned  first..
                if(available.empty())
                {
                    entity = static_cast<entity_type>(entities.size());

                    //succinctly place a bitset without any further info..
                    entities.emplace_back();
                }
                else
                {
                    //prioritize cache stack..
                    entity = available.top();
                    available.pop();
                }

                //enable last bitflag; must be set for valid(entity) to return true..
                entities[entity].set(validity_bit);
                return entity;
            }

            void destroy(entity_type entity)
            {
                assert(valid(entity));
                using accumulator_type = int[];
                accumulator_type accumulator = {0, (reset<Cs>(entity), 0)...};
                available.push(entity);
                entities[entity].reset();
                (void) accumulator;
            }

            // adds a component for an entity
            template<typename Comp, typename ... Args>
            Comp& assign(entity_type entity, Args&& ... args)
            {
                //ensure that entity is valid..
                assert(valid(entity));

                //@var entities holds a bitmask for each entity..
                //this either sets a bit to on or off for the given
                //component index..

                //component index list adhocly created by registry's variadic template parameters..
                //component index retrieved by get<C>()..
                entities[entity].set(ident<Cs...>.template get<Comp>());
                return pool.template construct<Comp>(entity, std::forward<Args>(args)...);
            }

            template<typename Comp>
            void remove(entity_type entity)
            {
                assert(valid(entity));
                entities[entity].reset(ident<Cs...>.template get<Comp>());
                pool.template destroy<Comp>(entity);
            }

            template<typename ... Comp>
            bool has(entity_type entity) const noexcept
            {
                assert(valid(entity));
                using accumulator_type = bool[];
                bool all = true;
                auto& mask = entities[entity];
                accumulator_type accumulator = {true, (all = all && mask.test(ident<Comp...>.template get<Comp>()))...};
                (void) accumulator;
                return all;
            }

            template<typename Comp>
            const Comp& get(entity_type entity) const noexcept
            {
                return pool.template get<Comp>(entity);
            }

            template<typename Comp>
            Comp& get(entity_type entity) noexcept
            {
                return pool.template get<Comp>(entity);
            }

            template<typename Comp, typename ... Args>
            Comp& replace(entity_type entity, Args&& ... args)
            {
                return (pool.template get<Comp>(entity) = Comp{std::forward<Args>(args)...});
            }

            template<typename Comp, typename ... Args>
            Comp& accomodate(entity_type entity, Args&& ... args)
            {
                assert(valid(entity));
                return (entities[entity].test(ident<Cs...>.template get<Comp>()) ?
                    this->template replace<Comp>(entity, std::forward<Args>(args)...) :
                    this->template assign <Comp>(entity, std::forward<Args>(args)...));
            }

            entity_type clone(entity_type from)
            {
                assert(valid(from));
                using accumulator_type = int[];
                auto to = create();
                accumulator_type accumulator = {0, (clone<Cs>(to, from), 0)...};
                (void) accumulator;
                return to;
            }

            template<typename Comp>
            Comp& copy(entity_type to, entity_type from)
            {
                return (pool.template get<Comp>(to) = pool.template get<Comp>(from));
            }

            void copy(entity_type to, entity_type from)
            {
                using accumulator_type = int[];
                accumulator_type accumulator = {0, (sync<Cs>(to, from), 0)...};
                (void) accumulator;
            }

            template<typename Comp>
            void reset(entity_type entity)
            {
                assert(valid(entity));
                if(entities[entity].test(ident<Cs...>.template get<Comp>()))
                    remove<Comp>(entity);
            }

            template<typename Comp>
            void reset()
            {
                for(entity_type entity = 0, last = entity_type(entities.size()); entity < last; ++entity)
                {
                    if(entities[entity].test(ident<Cs...>.template get<Comp>()))
                        remove<Comp>(entity);
                }
            }

            void reset()
            {
                entities.clear();
                while(!available.empty())
                    available.pop();
                pool.reset();
            }

            //iterate() defined for range-based for-loops..
            //class Arrow has begin() and end() defined..
            template<typename ... Comp>
            typename std::enable_if<(sizeof...(Comp)) == 1, arrow_type<Comp...>>::type
            iterate() noexcept
            {
                return arrow_type<Comp...>{pool};
            }

            template<typename ... Comp>
            typename std::enable_if<(sizeof...(Comp) > 1), arrow_type<Comp...>>::type
            iterate() noexcept
            {
                return {arrow_type<Comp...>{pool, entities.data()}};
            }
        private:
            std::vector<mask_type>  entities;
            std::stack<entity_type> available;
            pool_type               pool;
    };

    template<typename E, typename ... Comp>
    using StandardRegistry  = Registry<ComponentPool<E, Comp...>>;

    template<typename ... Comps>
    using DefaultRegistry   = Registry<ComponentPool<uint32_t, Comps...>>;
}

#endif // REGISTRY_H_INCLUDED
