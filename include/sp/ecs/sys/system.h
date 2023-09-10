#ifndef SP_BASE_SYSTEM_H
#define SP_BASE_SYSTEM_H
#include <sp/sp.h>
#include <memory>

namespace sp
{
    class Controller;
    class Observer;
    class SP_API System
    {
        public:
                        System(std::shared_ptr<Controller> ctrl);
                        System() = delete;
            virtual    ~System() = default;

            virtual void update(double dt) = 0;

        protected:
            std::shared_ptr<Controller> m_controller;
            std::shared_ptr<Observer>   m_observer;

    };
}

#endif // SYSTEM_H
