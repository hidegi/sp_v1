#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED
#include <sp/sp.h>
#include <sp/event/type_id.h>
#include <sp/utils/any.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>
#include <stack>
#include <map>

namespace sp
{
    using entity_type = uint32_t;
    template<typename C>
    class ComponentMap final
    {
        public:
            using component_type        = C;
            using Delegate              = std::pair<entity_type, component_type>;
            using iterator_type         = typename std::vector<Delegate>::iterator;
            using const_iterator_type   = typename std::vector<Delegate>::const_iterator;
            using size_type             = typename std::vector<Delegate>::size_type;
            using value_type            = Delegate;

        private:
            std::vector<size_type>      m_bucket;
            std::vector<Delegate>       m_clients;

        public:

            //arbitrary definition..
            SP_CONSTEXPR bool valid(entity_type entity) const noexcept
            {
                return (entity < m_bucket.size()) &&
                       (m_bucket[entity] < m_clients.size()) &&
                       (m_clients[m_bucket[entity]].first == entity);
            }

            explicit ComponentMap(size_type initial_size = 4096) noexcept
            {
                SP_ASSERT(initial_size > 0, "cannot allocate less-equal zero memory space");
                m_clients.reserve(initial_size);
            }

            C& getComponent(entity_type entity)
            {
                //SP_ASSERT(valid(entity), "cannot access by invalid index");
                static C c;
                if(!valid(entity))
                {
                    SP_PRINT_WARNING("could not find component for entity " << entity);
                    return c;
                }
                return m_clients[entity].second;
            }

            bool empty() const noexcept
            {
                return m_clients.empty();
            }

            size_type capacity() const noexcept
            {
                return m_clients.capacity();
            }

            iterator_type begin() noexcept
            {
                return m_clients.begin();
            }

            const_iterator_type cbegin() const noexcept
            {
                return m_clients.cbegin();
            }

            iterator_type end() noexcept
            {
                return m_clients.end();
            }

            const_iterator_type cend() const noexcept
            {
                return m_clients.cend();
            }

            bool has(entity_type entity) const noexcept
            {
                return valid(entity);
            }

            const component_type& get(entity_type entity) const noexcept
            {
                SP_ASSERT(valid(entity), "cannot access by invalid index");
                return m_clients[entity];
            }

            component_type& get(entity_type entity) noexcept
            {
                SP_ASSERT(valid(entity), "cannot access by invalid index");
                return m_clients[entity];
            }

            template<typename ... Args>
            void emplace(entity_type entity, Args&& ... args)
            {
                SP_ASSERT(!valid(entity), "cannot assign for already present entity");
                if(entity >= m_bucket.size())
                    m_bucket.resize(entity + 1);
                m_bucket[entity] = size_type{m_clients.size()};
                m_clients.push_back(std::make_pair( entity, C{std::forward<Args&&>(args)...}));

                /*
                static auto compare = [this](const Delegate& L, const Delegate& R)->bool
                {
                    return L.first < R.first;
                };
                std::sort(m_clients.begin(), m_clients.end(), compare);
                */
            }

            template<typename ... Args>
            void assign(entity_type entity, Args&& ... args)
            {
                //SP_ASSERT(valid(entity), "cannot access by invalid index");
                if(!valid(entity))
                {
                    emplace(entity, std::forward<Args>(args)...);
                    return;
                }
                /*
                auto it = std::find_if(m_clients.begin(), m_clients.end(),
                [&](const Delegate& delegate)->bool
                {
                    return delegate.first == entity;
                });
                if(it == m_clients.end())
                {
                    emplace(entity, std::forward<Args>(args)...);
                    return;
                }
                Delegate& delegate = *it;
                delegate.second = C{std::forward<Args>(args)...};
                */
                m_clients[m_bucket[entity]].second = C{std::forward<Args>(args)...};
            }


            void remove(entity_type entity)
            {
                SP_ASSERT(valid(entity), "cannot access by invalid index");
                /*
                auto it = std::find_if(m_clients.begin(), m_clients.end(),
                [&](const Delegate& delegate)->bool
                {
                    return delegate.first == entity;
                });
                if(it != m_clients.end())
                {
                    m_clients.erase(it);
                }
                */

                size_type last = m_clients.size() - 1;
                m_bucket[m_clients[last].first] = m_bucket[entity];
                m_clients[m_bucket[entity]].first  = m_clients[last].first;
                m_clients[m_bucket[entity]].second = std::move(m_clients[last].second);
                m_clients.pop_back();
            }

            void clear()
            {
                m_clients.clear();
            }


        private:
            iterator_type       from;
            iterator_type       to;
    };

    template<template<typename> class Map, typename C>
    class MapWrapper
    {
        private:
            using map_type              = Map<C>;
            using iterator_type         = typename map_type::iterator_type;
            using const_iterator_type   = typename map_type::iterator_type;
            using value_type            = typename map_type::value_type;
            using size_type             = typename map_type::size_type;
            using iterator_category     = std::input_iterator_tag;
        private:
            class MapIterator
            {
                private:

                public:
                    MapIterator(const_iterator_type b, const_iterator_type e) :
                        begin   {b},
                        end     {e}
                    {
                        if(begin != end)
                            ++(*this);
                    }

                    MapIterator& operator++() noexcept
                    {
                        ++begin;
                        while(begin != end) ++begin;
                        return *this;
                    }

                    MapIterator& operator++(int) noexcept
                    {
                        MapIterator original = *this;
                        return ++(*this), original;
                    }

                    bool operator==(const MapIterator& other) const noexcept
                    {
                        return begin == other.begin;
                    }

                    bool operator!=(const MapIterator& other) const noexcept
                    {
                        return !(*this == other);
                    }

                    value_type& operator*() noexcept
                    {
                        return *begin;
                    }

                    const value_type& operator*() const noexcept
                    {
                        return *begin;
                    }
                private:
                    iterator_type   begin;
                    iterator_type   end;
            };

            template<typename Com>
            void setSize(size_type& size) noexcept
            {
                auto sz = map.template size<Com>();
                if(sz < size)
                {
                    from = map.template begin();
                    to   = map.template end();
                }
            }
        public:
            explicit MapWrapper(map_type& m) noexcept :
                map  {m},
                from {m.template begin()},
                to   {m.template end()}
            {
            }

            const_iterator_type begin() const noexcept
            {
                return MapIterator{from, to};
            }

            iterator_type begin() noexcept
            {
                return const_cast<const MapWrapper*>(this)->begin();
            }

            const_iterator_type end() const noexcept
            {
                return MapIterator{to, to};
            }

            iterator_type end() noexcept
            {
                return const_cast<const MapWrapper*>(this)->end();
            }

            void reset() noexcept
            {
                from = map.begin();
                to   = map.end();
            }

            map_type iterate() noexcept
            {
                return map_type{map};
            }

        private:
            map_type&               map;
            const_iterator_type     from;
            const_iterator_type     to;
    };

    template<typename C>
    using DefaultMap = ComponentMap<C>;

    template<template<typename> class Map, typename C>
    using WrapperTyped = MapWrapper<Map, C>;

    template<typename C>
    using Wrapper = WrapperTyped<ComponentMap, C>;

    class Machine
    {
        private:

            template<typename C>
            using map_type      = ComponentMap<C>;

            template<typename C>
            using wrapper_type  = MapWrapper<ComponentMap, C>;

        private:
            template<typename C>
            sp::Any& proxy_create()
            {
                type_id_t id = type_id<map_type<C>>();
                auto it = m_id_to_map.find(id);
                if(it == m_id_to_map.end())
                {
                    auto&& any = sp::Any{map_type<C>{}};
                    try
                    {
                        any_cast<map_type<C>>(any);
                    }
                    catch(...)
                    {
                        SP_PRINT_WARNING("bad any cast at create..");
                    }

                    return m_id_to_map.emplace(std::make_pair(id, std::forward<Any>(any))).first->second;
                }

                return it->second;
            }

            template<class C>
            SP_CONSTEXPR bool is_component_valid()
            {
                static_assert(std::is_default_constructible<C>::value == true, "cannot have non-default-constructible component..");
                static_assert(std::is_copy_constructible<C>::value == true, "cannot have non-default-constructible component..");
                static_assert(std::is_move_constructible<C>::value == true, "cannot have non-default-constructible component..");
                return true;
            }
        public:
            typedef std::shared_ptr<Machine>            Ptr;
            typedef std::shared_ptr<const Machine>      ConstPtr;

            Machine()
            {
                m_available.clear();
                m_entities.clear();
            }

            bool valid(entity_type entity) const noexcept
            {
                return (entity < m_entities.size());
            }

            entity_type create() noexcept
            {
                entity_type entity;

                if(m_available.empty())
                {
                    entity = static_cast<entity_type>(m_entities.size());
                    m_entities.emplace_back();
                }
                else
                {
                    entity = m_available.back();
                    m_available.pop_back();
                }

                return entity;
            }

            template<typename C, typename ... Args>
            void emplace(entity_type entity, Args&& ... args)
            {
                is_component_valid<C>();
                SP_ASSERT(valid(entity), "cannot emplace invalid entity");
                sp::Any& map = proxy_create<C>();
                SP_ASSERT(!map.empty(), "failed to create component map");
                any_cast<map_type<C>>(&map)->emplace(entity, std::forward<Args>(args)...);
            }

            template<typename C, typename ... Args>
            void assign(entity_type entity, Args&& ... args)
            {
                is_component_valid<C>();
                SP_ASSERT(valid(entity), "cannot assign invalid entity");
                sp::Any& map = proxy_create<C>();
                SP_ASSERT(!map.empty(), "failed to create component map");
                any_cast<map_type<C>>(&map)->assign(entity, std::forward<Args>(args)...);
            }

            template<typename C>
            void remove(entity_type entity)
            {
                SP_ASSERT(valid(entity), "cannot remove invalid entity");
                sp::Any& map = proxy_create<C>();
                SP_ASSERT(!map.empty(), "failed to create component map");
                m_available.push_back(entity);

                any_cast<map_type<C>>(&map)->remove(entity);
            }

            template<typename C>
            bool has(entity_type entity) const noexcept
            {
                SP_ASSERT(valid(entity), "cannot query invalid entity");
                auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](entity_type id)
                {
                    return entity == id;
                });
                return it != m_entities.end();
            }

            template<typename C>
            C& getComponent(entity_type entity)
            {
                sp::Any& map = proxy_create<C>();
                return any_cast<map_type<C>>(&map)->getComponent(entity);
            }

            template<typename C>
            map_type<C>& iterate()
            {
                sp::Any& any = proxy_create<C>();
                map_type<C>* map = any_cast<map_type<C>>(&any);
                return *map;
            }

        private:
            std::vector<sp::Any>                    m_maps;
            std::map<type_id_t, sp::Any>            m_id_to_map;

            std::vector<entity_type>                m_entities;
            std::vector<entity_type>                m_available;
    };
}

#endif
