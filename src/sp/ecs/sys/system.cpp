#include <sp/ecs/sys/system.h>
#include <sp/sp_controller.h>
#include <sp/event/observer.h>
#include <algorithm>

namespace sp
{
    System::System(std::shared_ptr<Controller> ctrl) :
        m_controller    {ctrl}
    {
        m_observer = m_controller->createObserver();
    }
    /*
    System::System()
    {
    }

    System::~System()
    {
    }

    const ComponentSet& getRequiredComponents() const
    {
        return m_requirements;
    }

    void insertRequiredComponent(componentSet&& req)
    {
        m_requirements = std::move(req);
    }

    bool System::addEntity(uint32_t id)
    {
        m_entities.insert(id).second;
    }

    bool System::hasEntity(uint32_t id)
    {
        //auto it = std::find(m_entities.begin(), m_entities.end(), id);
        return (m_entities.find(id) != m_entities.end());
    }

    bool System::removeEntity(uint32_t id)
    {
        /*
        auto lmbd = [&](uint32_t other)->bool{return id == other;};
        auto it = std::find_if(m_entities.begin(), m_entities.end(), lmbd);
        if(it != m_entities.end())
            m_entities.erase(it);
        *
        return (m_entities.erase(id) > 0);
    }
    */
}
