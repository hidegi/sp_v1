#include <sp/exsp/ex/perks.h>
#include <sp/exsp/ex/event.h>

//immediate return..
Flag AnyPassPerk::onPrePostponeEvent(sp::bus::PostponeHelper& postponeCall)
{
    printf("any pass perk..\n");
    postponeCall.postponeCallback(*_passToBus, std::move(postponeCall.event));
    return Flag::PostponeCancel;
}

PerkEventBus::RegisterHelper PerkEventBus::addPerk(std::shared_ptr<Perk> perk)
{
    auto* local = perk.get();
    _perks.push_back(std::move(perk));
    return RegisterHelper(this, local);
}

bool PerkEventBus::postponeEvent(sp::bus::PostponeHelper& postponeCall)
{
    for(const auto& onPrePostpone : _onPrePostpone)
    {
        if(onPrePostpone(postponeCall) == Flag::PostponeCancel)
            return false;
    }

    if(EventBus::postponeEvent(postponeCall))
    {
        for(const auto& onPostPostpone : _onPostPostpone)
        {
            if(onPostPostpone(postponeCall) == Flag::PostponeCancel)
                break;
        }
        return true;
    }
    return false;
}

Flag TagPerk::onPrePostponeEvent(sp::bus::PostponeHelper& postponeCall)
{
    auto found = _eventsToWrap.find(postponeCall.eventID);
    if( found != _eventsToWrap.end())
    {
        if(found->second)
        {
            //postponeCall.event ... ok..
            printf("here..\n");
            found->second(postponeCall.event);
        }
        return Flag::PostponeCancel;
    }
    return Flag::PostponeContinue;
}

bool WaitPerk::wait()
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lock(_waitMutex);
    if(_hasEvents)
    {
        _hasEvents = false;
        return true;
    }

    _eventWaiting.wait(lock, [this](){return _hasEvents;});
    if(_hasEvents)
    {
        _hasEvents = false;
        return true;
    }
    return false;
}

bool WaitPerk::waitFor(const std::chrono::milliseconds timeout)
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lock(_waitMutex);
    if(_hasEvents)
    {
        _hasEvents = false;
        return true;
    }

    if(_eventWaiting.wait_for(lock, timeout, [this](){return _hasEvents;}))
    {
        _hasEvents = false;
        return true;
    }

    return false;
}

Flag WaitPerk::onPostponeEvent(sp::bus::PostponeHelper&)
{
    {
        std::lock_guard<std::mutex> lock(_waitMutex);
        _hasEvents = true;
    }

    _eventWaiting.notify_one();
    return Flag::PostponeContinue;
}
