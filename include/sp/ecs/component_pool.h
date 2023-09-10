#ifndef COMPONENT_POOL_H_INCLUDED
#define COMPONENT_POOL_H_INCLUDED
#include <sp/sp.h>
#include <sp/ecs/spec.h>
#include <utility>
#include <vector>

namespace sp
{
    //forward declaration of complete form..
    template<typename, typename, typename ... >
    class ComponentPool;

    //recursion breaker..
    //each component type will instantiate this template class specifically..
    //therefore, each component class will have own memory stored for each entity..
    template<typename E, typename C>
    class ComponentPool<E, C>
    {
        public:
            using component_type        = C;
            using entity_type           = E;            //uint32_t..
            using pos_type              = entity_type;  //uint32_t..
            using size_type             = typename std::vector<component_type>::size_type;
            using iterator_type         = typename std::vector<entity_type>::iterator;
            using const_iterator_type   = typename std::vector<entity_type>::const_iterator;

        private:
            //since entity-ids are in ascending order, they are used against validity checks..
            //for an entity to be valid, the following applies:
            SP_CONSTEXPR bool valid(entity_type entity) const noexcept
            {
                return (entity < reverse.size()) && (reverse[entity] < direct.size()) && (direct[reverse[entity]] == entity);
            }

        public:
            //access policy:
            //  data:       data[reverse[entity]]
            //  direct:     direct[reverse[entity]]
            //  reverse:    reverse[entity] = direct.size()

            //the following doesn't have to apply: reverse[entity] = entity..
            //data is the actual holder for each component an entity may have..
            explicit ComponentPool(size_type initial_size = 4096) noexcept
            {
                assert(initial_size > 0);
                data.reserve(initial_size);
            }

            ComponentPool(ComponentPool&&) = default;
           ~ComponentPool() noexcept {assert(empty());}

            bool empty() const noexcept
            {
                return data.empty();
            }

            size_type capacity() const noexcept
            {
                return data.capacity();
            }

            size_type size() const noexcept
            {
                return data.size();
            }

            iterator_type begin() noexcept
            {
                return direct.begin();
            }

            const_iterator_type cbegin() const noexcept
            {
                return direct.begin();
            }

            iterator_type end() noexcept
            {
                return direct.end();
            }

            const_iterator_type cend() const noexcept
            {
                return direct.end();
            }

            bool has(entity_type entity) const noexcept
            {
                return valid(entity);
            }

            const component_type& get(entity_type entity) const noexcept
            {
                assert(valid(entity));
                return data[reverse[entity]];
            }

            component_type& get(entity_type entity) noexcept
            {
                return const_cast<component_type&>(const_cast<const ComponentPool*>(this)->get(entity));
            }

            //construct a component for type C by the given variadic parameters @param Args..
            template<typename ... Args>
            component_type& construct(entity_type entity, Args&& ... args)
            {
                assert(!valid(entity));

                if(entity >= reverse.size())
                {
                    //actually bad, since some unused memory is being reserved,
                    //in order to match a mutual component size for every
                    //template class instantiation..
                    reverse.resize(entity + 1);
                }

                //at best case (reverse[entity] = entity)..
                reverse[entity] = pos_type(direct.size());
                direct.emplace_back(entity);
                data.push_back({std::forward<Args>(args)...});
                return data.back();
            }

            void destroy(entity_type entity)
            {
                assert(valid(entity));
                auto last = direct.size() - 1;
                reverse[direct[last]]   = reverse[entity];
                direct[reverse[entity]] = direct[last];
                data  [reverse[entity]] = std::move(data[last]);
                direct.pop_back();
                data.pop_back();
            }

            void reset()
            {
                data.clear();
                reverse.resize(0);
                direct.clear();
            }
        private:
            std::vector<component_type>     data;

            //direct and reverse are exclusively inversely ordered..
            std::vector<pos_type>           reverse;
            std::vector<entity_type>        direct;
    };

    //for every new component C, the base cst is invoked, instantiating a template class
    //for any new component C(s)..

    //the latter class only holds access functions for the former class..
    //note that this class here has no members!!
    template<typename E, typename C, typename ... Cs>
    class ComponentPool : ComponentPool<E, C>, ComponentPool<E, Cs>...
    {
        template<typename Comp>
        using Pool = ComponentPool<E, Comp>;

        public:
            using entity_type           = typename Pool<C>::entity_type;
            using pos_type              = typename Pool<C>::pos_type;
            using size_type             = typename Pool<C>::size_type;
            using iterator_type         = typename Pool<C>::iterator_type;
            using const_iterator_type   = typename Pool<C>::const_iterator_type;

            explicit ComponentPool(size_type initial_size = 4096) noexcept :
                ComponentPool<E, C>{initial_size}, ComponentPool<E, Cs>{initial_size}...
            {
                assert(initial_size > 0);
            }

            ComponentPool(const ComponentPool&) = delete;
            ComponentPool(ComponentPool&&) = delete;
            ComponentPool& operator=(const ComponentPool&) = delete;
            ComponentPool& operator=(ComponentPool&&) = delete;

            template<typename Comp>
            bool empty() const noexcept
            {
                return Pool<Comp>::empty();
            }

            template<typename Comp>
            size_type capcacity() const noexcept
            {
                return Pool<Comp>::capacity();
            }

            template<typename Comp>
            size_type size() const noexcept
            {
                return Pool<Comp>::size();
            }

            template<typename Comp>
            iterator_type begin() noexcept
            {
                return Pool<Comp>::begin();
            }

            template<typename Comp>
            const_iterator_type cbegin() const noexcept
            {
                return Pool<Comp>::cbegin();
            }

            template<typename Comp>
            iterator_type end() noexcept
            {
                return Pool<Comp>::end();
            }

            template<typename Comp>
            const_iterator_type cend() const noexcept
            {
                return Pool<Comp>::cend();
            }

            template<typename Comp>
            bool has(entity_type entity) const noexcept
            {
                return Pool<Comp>::has(entity);
            }

            template<typename Comp>
            const Comp& get(entity_type entity) const noexcept
            {
                return Pool<Comp>::get(entity);
            }

            template<typename Comp>
            Comp& get(entity_type entity) noexcept
            {
                return const_cast<Comp&>(const_cast<const ComponentPool*>(this)->get<Comp>(entity));
            }

            template<typename Comp, typename ... Args>
            Comp& construct(entity_type entity, Args&& ... args) noexcept
            {
                return Pool<Comp>::construct(entity, std::forward<Args>(args)...);
            }

            template<typename Comp>
            void destroy(entity_type entity)
            {
                Pool<Comp>::destroy(entity);
            }

            //set current pool to a default state..
            template<typename Comp>
            void reset()
            {
                Pool<Comp>::reset();
            }

            //set all pools to a default state..
            void reset()
            {
                using accumulator_type = int[];
                Pool<C>::reset();

                accumulator_type accumulator =
                {
                    (Pool<Cs>::reset(), 0)...
                };
                (void) accumulator;
            }
    };

}


#endif // COMPONENT_POOL_H_INCLUDED
