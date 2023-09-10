#ifndef SP_LISTENER_H
#define SP_LISTENER_H
#include <sp/sp_event.h>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

namespace sp
{
    class Listener
    {
        friend class Subscription;

        public:
            virtual            ~Listener() = default;
            void                setSystemCallback(SP_SystemEvent event, std::function<void(SP_Detail*)> fn);

        private:
            void                notify(SP_Detail* detail) const;
            //void                notify(UserEvent* event) const;

            typedef std::vector<std::function<void(SP_Detail*)>>    CallbackVector;
            std::unordered_map<long int, CallbackVector>      m_callbacks;

    };
}
#endif
