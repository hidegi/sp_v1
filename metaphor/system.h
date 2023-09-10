#ifndef SP_BASE_SYSTEM_H
#define SP_BASE_SYSTEM_H
#include <sp/exsp/type_id.h>
#include <sp/exsp/listener_sprb.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <set>

namespace sp
{
    /*
    class SystemBus;
    class System
    {
        public:
            typedef std::set<sp::inter::type_id_t> ComponentSet;

                                System() = delete;
                                System(std::shared_ptr<SystemBus> bus);
            virtual            ~System();
            const ComponentSet& getRequiredComponents() const;

            virtual void        update(double dt) = 0;
            bool                addEntity(uint32_t id);
            bool                hasEntity(uint32_t id);
            bool                removeEntity(uint32_t id);

        protected:
            void                insertRequiredComponent(componentSet&& req);

            std::set<uint32_t>      m_entities;
            ComponentSet            m_requirements;
            uint64_t                m_id;
            sp::BusListener         m_listener;
    };
    */
}

#endif // SYSTEM_H
