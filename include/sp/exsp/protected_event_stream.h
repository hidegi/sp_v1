#ifndef PROTECTED_EVENT_STREAM_H_INCLUDED
#define PROTECTED_EVENT_STREAM_H_INCLUDED
#include <sp/exsp/event_stream.h>
#include <sp/utils/shared_mutex.h>
#include <sp/utils/any.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <atomic>
#include <vector>
#include <mutex>

namespace sp::bus::stream
{
    //protected, since more than one thread may be posing events..
    template<typename Event, typename CallbackReturn = void, typename ... ExtraArgs>
    class ProtectedEventStream : public EventStream
    {
        using Callback = std::function<CallbackReturn(const Event&, ExtraArgs...)>;
        private:
            std::vector<uint32_t>                       _listenerIDs;
            std::vector<Event>                          _queue;
            std::vector<Callback>                       _callbacks;
            std::atomic<bool>                           _isProcessing{false};
            std::vector<std::pair<uint32_t, Callback>>  _waiting;
            sp::shared_mutex                            _mutexEvent;
            sp::shared_mutex                            _mutexCallbacks;

            //for all listeners being stalled, will flush for add/remove..
            void flushWaitingElements()
            {
                //cannot be processing if invoked, therefore assert..
                assert(!_isProcessing);
                std::lock_guard<shared_mutex> writeGuard{_mutexCallbacks};
                if(_waiting.empty()) return;
                for(auto&& element : _waiting)
                {
                    if(element.second)
                    {
                        rawAddListener(element.first, std::move(element.second));
                    }
                    else
                    {
                        rawRemoveListener(element.first);
                    }
                }

                _waiting.clear();
            }

            //directly adds a listener from the list, if current stream is not processing..
            bool rawAddListener(const uint32_t listenerID, Callback&& callback)
            {
                auto found = std::find(_listenerIDs.begin(), _listenerIDs.end(), listenerID);
                if(found != _listenerIDs.end())
                {
                    throw SP_EXCEPTION("already added listener for event " + typeid(Event).name());
                }

                _callbacks.push_back(std::forward<Callback>(callback));
                _listenerIDs.push_back(listenerID);
                assert(_listenerIDs.size() == _callbacks.size());
                return true;
            }

            //directly removes a listener from the list, if current stream is not processing..
            bool rawRemoveListener(const uint32_t listenerID)
            {
                auto found = std::find(_listenerIDs.begin(), _listenerIDs.end(), listenerID);
                if(found == _listenerIDs.end())
                {
                    return false;
                }

                const auto index = std::distance(_listenerIDs.begin(), found);
                _listenerIDs.erase(found);
                _callbacks.erase(std::next(_callbacks.begin(), index));
                assert(_listenerIDs.size() == _callbacks.size());
                return true;
            }
        public:

            //direct push for an event..
            void postpone(sp::Any event) override
            {
                auto curr_event = any_cast<Event>(event);
                std::lock_guard<shared_mutex> writeGuard{_mutexEvent};
                _queue.push_back(std::move(curr_event));
            }

            //process a limited amount of listeners for a given time span..
            std::size_t process(const size_t limit) override
            {
                std::vector<Event> processEvents;
                {
                    std::lock_guard<shared_mutex> writeGuard(_mutexEvent);
                    if(limit >= _queue.size())
                    {
                        processEvents.reserve(_queue.size());
                        std::swap(processEvents, _queue);
                    }
                    else
                    {
                        const auto countElements = std::min(limit, _queue.size());
                        processEvents.reserve(countElements);
                        auto begin = _queue.begin();
                        auto end = std::next(begin, countElements);

                        std::move(begin, end, std::back_inserter(processEvents));
                        _queue.erase(begin, end);
                    }
                }

                for(const auto& event : processEvents)
                {

                    _isProcessing = true;
                    for(auto& callback : _callbacks)
                    {
                        callback(event);
                    }
                    _isProcessing = false;
                    flushWaitingElements();
                }

                return processEvents.size();
            }

            //if currently not processing, will add the listener to the list,
            //otherwise, being stalled on list..
            bool addListener(const uint32_t listenerID, sp::Any callback) override
            {
                std::lock_guard<shared_mutex> writeGuard(_mutexCallbacks);
                auto curr_callback = any_cast<Callback>(callback);
                if(_isProcessing)
                {
                    _waiting.emplace_back(listenerID, std::move(curr_callback));
                    return true;
                }

                return rawAddListener(listenerID, std::move(curr_callback));
            }

            //if currently not processing, will remove the listener from the list,
            //otherwise, being stalled on list..
            bool removeListener(const uint32_t listenerID) override
            {
                std::lock_guard<shared_mutex> writeGuard(_mutexCallbacks);
                if(_isProcessing)
                {
                    _waiting.emplace_back(listenerID, Callback{});
                    return true;
                }

                return rawRemoveListener(listenerID);
            }

            bool hasEvents() const
            {
#if SP_COMPILE_VERSION >= 17
                std::shared_lock readGuard(_mutexEvent);
#else
                s_lock readGuard(_mutexEvent);
#endif
                return !_queue.empty();
            }
    };
}

#endif // PROTECTED_EVENT_STREAM_H_INCLUDED
