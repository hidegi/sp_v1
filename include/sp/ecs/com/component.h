#ifndef SP_COMPONENT_H_INCLUDED
#define SP_COMPONENT_H_INCLUDED

namespace sp
{
    typedef uint32_t Entity;
    class IComponent
    {
        public:
        typedef std::unique_ptr<Component>          Ptr;
        typedef std::unique_ptr<const Component>    ConstPtr;
    };

    class Component
    {
        public:
            virtual ~Component() {}
    };

    template<typename C>
    class CompContainer : public IComponent
    {
        static_assert(std::is_base_of<Component, C>::value, "type C must be derived from Component..");

        public:
                            CompContainer() {}
                           ~CompContainer() {}

            SP_CONSTEXPR bool add(uint32_t ent, C&& comp)
            {
                return m_container.insert(std::make_pair(ent, comp));
            }

            SP_CONSTEXPR bool remove(uint32_t ent)
            {
                return (0 < m_container.erase(ent));
            }

            SP_CONSTEXPR bool has(uint32_t ent) const
            {
                return (m_container.find(ent) != m_container.end());
            }

            SP_CONSTEXPR C& get(uint32_t ent)
            {
                return m_container.at(ent);
            }

            SP_CONSTEXPR C extract(uint32_t ent)
            {
                C comp = std::move(m_container.at(ent));
                m_container.remove(ent);
                return comp;
            }

        private:
            std::unordered_map<uint32_t, C> m_container;

    };
}

#endif // COMPONENT_H_INCLUDED
