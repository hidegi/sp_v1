#ifndef MOTEX_DISPLAY_H_INCLUDED
#define MOTEX_DISPLAY_H_INCLUDED
#include <map>
#include <string>
#include <sp/exsp/event_bus.h>
#include <sp/exsp/ex/event.h>
#include <sp/exsp/listener_sprb.h>

class Gui
{
    public:
        Gui(const std::shared_ptr<sp::EventBus>& bus);
        void draw();

    private:
        sp::BusListener                 _listener;
        std::map<std::string, int>      _gold_sack;
};

Gui::Gui(const std::shared_ptr<sp::EventBus>& bus) :
    _listener{bus}
{
    _listener.listen<NewTeamMember>([this](const NewTeamMember& event){
        _gold_sack.emplace(event.memberName, 0);});
    _listener.listen<TagEvent<GoldUpdate>>([this](const TagEvent<GoldUpdate>& event){
        auto found = _gold_sack.find(event.tag);
        if(found != _gold_sack.end())
            found->second = event.data.goldCount;
    });
}

void Gui::draw()
{
    printf("----------------------------------\n");
    for(const auto& player : _gold_sack)
    {
        printf("name: %s - gold: %d\n", player.first.c_str(), player.second);
    }
    printf("----------------------------------\n");
}


#endif // DISPLAY_H_INCLUDED
