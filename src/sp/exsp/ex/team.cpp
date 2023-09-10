#include <sp/exsp/ex/team.h>
#include <sp/exception.h>
#include <sp/exsp/ex/perks.h>

using namespace sp;

Team::Team(std::shared_ptr<sp::EventBus> bus) :
    _bus(std::move(bus))
{
}

Character& Team::getMember(const std::string& name)
{
    auto found = std::find(_names.begin(), _names.end(), name);
    if(found == _names.end())
    {
        throw SP_EXCEPTION("no such member");
    }
    return _squad.at(std::distance(_names.begin(), found));
}

void Team::addPlayer(const std::string& name)
{
    auto characterBus = std::make_shared<PerkEventBus>();
    {
        auto tagPerk = std::make_unique<TagPerk>(name, _bus.get());
        tagPerk->wrapTag<TagEvent<GoldUpdate>>();
        characterBus->addPerk(std::move(tagPerk)).registerPrePostpone(&TagPerk::onPrePostponeEvent);
    }

    characterBus->addPerk(std::make_unique<AnyPassPerk>(_bus)).registerPrePostpone(&AnyPassPerk::onPrePostponeEvent);
    _squad.emplace_back(Character(characterBus));
    _names.push_back(name),
    _bus->postpone(NewTeamMember{name});
}
