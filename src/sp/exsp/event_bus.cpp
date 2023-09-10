#include <sp/exsp/event_bus.h>

namespace sp
{
    size_t EventBus::processLimit(const size_t limit)
    {
        size_t processCount{0};
        std::lock_guard<sp::shared_mutex> writeGuardProcess(_mutexProcess);
        std::vector<std::unique_ptr<bus::stream::EventStream>> eventStreams;
        {
            std::lock_guard<sp::shared_mutex> writeGuard(_mutexStreams);
            std::swap(eventStreams, _eventStreams);
        }

        for(auto& eventStream : eventStreams)
        {
            const size_t runProcessCount = eventStream->process(limit);
            processCount += runProcessCount;
            if(processCount >= limit)
                break;
        }
        {
            std::lock_guard<sp::shared_mutex> writeGuard(_mutexStreams);
            if(!_eventStreams.empty())
            {
                std::move(_eventStreams.begin(), _eventStreams.end(), std::back_inserter(eventStreams));
            }
            std::swap(eventStreams, _eventStreams);

            if(_eventStreams.size() != _eventToStream.size())
            {
                auto removeFrom = std::remove_if(_eventStreams.begin(), _eventStreams.end(), [this](const auto& eventStream)
                {
                    for(const auto& element : _eventToStream)
                    {
                        if(element.second == eventStream.get())
                            return false;
                    }
                    return true;
                });

                assert(removeFrom != _eventStreams.end());
                _eventStreams.erase(removeFrom, _eventStreams.end());
            }
        }
        return processCount;
    }

    bus::stream::EventStream* EventBus::findStream(const inter::type_id_t eventID) const
    {
#if SP_COMPILE_VERSION >= 17
                std::shared_lock readGuard(_mutexStreams);
#else
                s_lock readGuard(_mutexStreams);
#endif
        return findStreamUnsafe(eventID);
    }

    void EventBus::unlistenAll(const uint32_t listenerID)
    {
#if SP_COMPILE_VERSION >= 17
                std::shared_lock readGuard(_mutexStreams);
#else
                s_lock readGuard(_mutexStreams);
#endif
        for(const auto& eventStream : _eventToStream)
        {
            eventStream.second->removeListener(listenerID);
        }
    }

    bus::stream::EventStream* EventBus::findStreamUnsafe(const inter::type_id_t eventID) const
    {
        auto lookup = _eventToStream.find(eventID);
        return lookup != _eventToStream.end() ? lookup->second : nullptr;
    }

    bus::stream::EventStream* EventBus::obtainStream(const inter::type_id_t eventID,
                                                     bus::CreateStreamCallback callback)
    {
        std::lock_guard<sp::shared_mutex> writeGuard(_mutexStreams);
        auto* found = findStreamUnsafe(eventID);
        if(found)
            return found;
        else
        {
            auto stream = callback();
            _eventStreams.push_back(std::move(stream));
            _eventToStream[eventID] = _eventStreams.back().get();
            return _eventStreams.back().get();
        }
    }

    bool EventBus::postponeEvent(bus::PostponeHelper& postponeCall)
    {
        auto* eventStream = obtainStream(postponeCall.eventID, postponeCall.createStreamCallback);
        eventStream->postpone(std::move(postponeCall.event));
        return true;
    }

    bus::stream::EventStream* EventBus::listen(const uint32_t, const inter::type_id_t eventID,
                                               bus::CreateStreamCallback callback)
    {
        auto* eventStream = obtainStream(eventID, callback);
        return eventStream;
    }

    void EventBus::unlisten(const uint32_t listenerID, const inter::type_id_t eventID)
    {
        bus::stream::EventStream* eventStream = findStream(eventID);
        if(eventStream)
        {
            eventStream->removeListener(listenerID);
        }
    }
}
