#ifndef MOTEX_CHARACTER_H_INCLUDED
#define MOTEX_CHARACTER_H_INCLUDED
#include <sp/exsp/event_bus.h>
#include <sp/exsp/ex/event.h>
#include <memory>
#include <string>
using namespace exsp;

class Character
{
    public:
        Character(std::shared_ptr<sp::EventBus> bus);
        void pickGold(int goldCount);
        void damage(int amount);

    private:
        std::shared_ptr<sp::EventBus> _bus;
        int _sackGold   = 0;
        int _health     = 100;
        int _iq         = 200;
};

#endif // CHARACTER_H_INCLUDED
