#ifndef MOTEX_TEAM_H
#define MOTEX_TEAM_H
#include <sp/exsp/ex/character.h>

class PerkEventBus;
class Team
{
    public:
        Team(std::shared_ptr<sp::EventBus> bus);
        void addPlayer(const std::string& name);
        Character& getMember(const std::string& name);

    private:
        std::vector<std::string> _names;
        std::vector<Character>   _squad;

        std::shared_ptr<sp::EventBus> _bus;
};

#endif // TEAM_H
