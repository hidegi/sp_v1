#ifndef SP_EVENT_BUS_H
#define SP_EVENT_BUS_H

#include <sp/utils/shared_mutex.h>
#include <sp/utils/any.h>
#include <sp/exsp/bus.h>
#include <iterator>
#include <limits>
#include <memory>
#include <map>

namespace sp
{
    class SP_API EventBus : public sp::bus::Bus
    {
        template<typename>
        friend class inter::ListenerAttorney;
        public:
            //actual interface for updates..
            size_t process() override
            {
                //an arbitrary amount of max-listeners being processed for one call..
                return processLimit(std::numeric_limits<size_t>::max());
            }

            size_t processLimit(const size_t limit)
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

        protected:
            /*
            bus::stream::EventStream* obtainStream(inter::type_id_t eventID,
                                                   bus::CreateStreamCallback callback);
            bool postponeEvent(bus::PostponeHelper& postponeCall) override;
            bus::stream::EventStream* findStream(inter::type_id_t eventID) const;

            void unlistenAll(uint32_t listenerID) override;
            bus::stream::EventStream* listen(uint32_t listenerID, inter::type_id_t eventID,
                                             bus::CreateStreamCallback callback) override;
            void unlisten(uint32_t listenerID, inter::type_id_t eventID) override;
            */



            bus::stream::EventStream* findStream(const inter::type_id_t eventID) const
            {
        #if SP_COMPILE_VERSION >= 17
                        std::shared_lock readGuard(_mutexStreams);
        #else
                        s_lock readGuard(_mutexStreams);
        #endif
                return findStreamUnsafe(eventID);
            }

            void unlistenAll(const uint32_t listenerID)
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



            bus::stream::EventStream* obtainStream(const inter::type_id_t eventID,
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

            bool postponeEvent(bus::PostponeHelper& postponeCall)
            {
                auto* eventStream = obtainStream(postponeCall.eventID, postponeCall.createStreamCallback);
                eventStream->postpone(std::move(postponeCall.event));
                return true;
            }

            bus::stream::EventStream* listen(const uint32_t, const inter::type_id_t eventID,
                                                       bus::CreateStreamCallback callback)
            {
                auto* eventStream = obtainStream(eventID, callback);
                return eventStream;
            }

            void unlisten(const uint32_t listenerID, const inter::type_id_t eventID)
            {
                bus::stream::EventStream* eventStream = findStream(eventID);
                if(eventStream)
                {
                    eventStream->removeListener(listenerID);
                }
            }
        private:
            bus::stream::EventStream* findStreamUnsafe(const inter::type_id_t eventID) const
            {
                auto lookup = _eventToStream.find(eventID);
                return lookup != _eventToStream.end() ? lookup->second : nullptr;
            }

            mutable sp::shared_mutex        _mutexStreams;
                    sp::shared_mutex        _mutexProcess;
            std::vector<std::unique_ptr<bus::stream::EventStream>>           _eventStreams;
            std::map<inter::type_id_t, bus::stream::EventStream*>     _eventToStream;


    };
}

#endif // EVENT_BUS_H
