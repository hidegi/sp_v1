#include <sp/exsp/ex/character.h>
Character::Character(std::shared_ptr<sp::EventBus> bus) :
    _bus(bus)
{
}

void Character::pickGold(int goldCount)
{
    _sackGold += goldCount;
    _bus->postpone(GoldUpdate{_sackGold});
}

void Character::damage(int amount)
{
    _health -= amount;
}
