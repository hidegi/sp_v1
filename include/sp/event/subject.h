#ifndef SUBJECT_H_INCLUDED
#define SUBJECT_H_INCLUDED
#include <sp/sp.h>
#include <sp/event/type_id.h>
#include <sp/utils/shared_mutex.h>
#include <sp/event/event_ex.h>

namespace sp
{
    template<typename E>
    using Callback = std::function<void(const E&)>;

    class SP_API IStream
    {
        public:
            virtual ~IStream() {}
            virtual bool emplace_observer(uint32_t name, sp::Any callback) = 0;
            virtual bool remove_observer(uint32_t name) = 0;
            virtual void process() = 0;
            virtual void dispatch(sp::Any) = 0;
    };

    export template<typename E, typename _ = void>
    class SP_API EventStream : public IStream
    {
        public:
            using Callback = std::function<void(const E&)>;

        private:
            using DelegateResolve = std::pair<uint32_t, Callback>;
            void flushDanglingEvents()
            {
                std::lock_guard<sp::shared_mutex> writeGuard(m_motex_callbacks);
                if(m_processing || (m_pending_queue.empty() && m_pending_events.empty())) return;
                for(auto && delegate : m_pending_queue)
                {
                    if(delegate.second)
                    {
                        emplace_directly(delegate.first, std::forward<Callback>(delegate.second));
                    }
                    else
                    {
                        remove_directly(delegate.first);
                    }
                }

                for(auto && event : m_pending_events)
                {
                    m_event_queue.push_back(std::forward<E>(event));
                }

                m_event_queue.clear();
                m_pending_queue.clear();
            }

            bool emplace_directly(const uint32_t observerID, Callback&& cb)
            {
                auto it = std::find(m_observers.begin(), m_observers.end(), observerID);
                if(it != m_observers.end())
                {
                    SP_PRINT_WARNING("observer has been already added to this event stream");
                    return false;
                }

                m_callbacks.push_back(std::forward<Callback>(cb));
                m_observers.push_back(observerID);

                SP_ASSERT(m_callbacks.size() == m_observers.size(), "mismatch occurred");
                return true;
            }

            bool remove_directly(const uint32_t observerID)
            {

                auto it = std::find(m_observers.begin(), m_observers.end(), observerID);
                if(it == m_observers.end())
                {
                    SP_PRINT_WARNING("observer has not been found in this event stream");
                    return false;
                }

                auto index = std::distance(m_observers.begin(), it);
                m_observers.push_back(observerID);
                m_callbacks.erase(std::next(m_callbacks.begin(), index));

                SP_ASSERT(m_callbacks.size() == m_observers.size(), "mismatch occurred");
                return true;
            }

        public:

            EventStream() :
                m_processing{false}
            {
            }

            void dispatch(sp::Any any)
            {
                emplace_event(std::move(any));
            }
            bool emplace_event(sp::Any&& any)
            {
                std::lock_guard<sp::shared_mutex> writeGuard(m_motex_events);
                if(!m_processing)
                {
                    m_event_queue.push_back(std::forward<E>(any_cast<E>(any)));
                    return true;
                }
                m_pending_events.push_back(std::forward<E>(any_cast<E>(any)));
                return true;
            }

            bool emplace_observer(const uint32_t observerID, sp::Any cb)
            {

                std::lock_guard<shared_mutex> writeGuard(m_motex_callbacks);
                auto callback = any_cast<Callback>(cb);
                if(m_processing)
                {
                    m_pending_queue.push_back({observerID, std::move(callback)});
                    return true;
                }
                return emplace_directly(observerID, std::move(callback));
            }

            bool remove_observer(const uint32_t observerID)
            {
                std::lock_guard<shared_mutex> writeGuard(m_motex_callbacks);
                if(m_processing)
                {
                    m_pending_queue.push_back({observerID, nullptr});
                    return true;
                }

                return remove_directly(observerID);
            }

            virtual void process() override
            {
                {
                    std::lock_guard<sp::shared_mutex>   writeGuard(m_motex_events);
                    m_processing = true;
                    for(auto& event : m_event_queue)
                    {
                        for(auto& cb : m_callbacks)
                        {
                            cb(event);
                        }
                    }
                    m_processing = false;
                }

                m_event_queue.clear();
                flushDanglingEvents();
            }



        private:
            bool                                        m_processing{false};
            std::vector<DelegateResolve>                m_pending_queue;
            std::vector<E>                              m_pending_events;

            std::vector<Callback>                       m_callbacks;
            std::vector<E>                              m_event_queue;
            std::vector<uint32_t>                       m_observers;
            sp::shared_mutex                            m_motex_events;
            sp::shared_mutex                            m_motex_callbacks;
    };

    export template<typename E>
    using DefaultStream        = EventStream<E>;
    using StreamCreateCallback = std::unique_ptr<IStream>(*const)();

    template<typename E>
    std::unique_ptr<IStream> SP_API createEventStream()
    {
        return std::make_unique<DefaultStream<E>>();
    }

    class SP_API Subject
    {
        private:
            //every event will have an own event stream..
            std::vector<std::unique_ptr<IStream>>       m_event_streams;
            std::map<type_id_t, IStream*>               m_event_to_stream;
            std::atomic<uint32_t>                       m_generator{0};
            sp::shared_mutex                            m_motex_streams;

        private:

            IStream* findStreamRaw(const type_id_t id) const
            {
                auto it = m_event_to_stream.find(id);
                return (it == m_event_to_stream.end()) ? nullptr : it->second;
            }

            IStream* findStream(const type_id_t id, StreamCreateCallback callback)
            {
#if SP_COMPILE_VERSION >= 17
                std::shared_lock readGuard(m_motex_streams);
#else
                s_lock readGuard(m_motex_streams);
#endif
                IStream* finder = findStreamRaw(id);
                if(!finder)
                {

                    auto stream = callback();
                    m_event_streams.push_back(std::move(stream));
                    finder = m_event_streams.back().get();
                    m_event_to_stream.emplace(std::make_pair(type_id_t(id), finder));
                }

                return finder;
            }
        public:
            typedef std::shared_ptr<Subject>            Ptr;
            typedef std::shared_ptr<const Subject>      ConstPtr;
            template<typename E>
            void registerObserver(const uint32_t name, Callback<E>&& callback)
            {
                //printf("registering observer..\n");
                static_assert(is_event_valid<E>(), "invalid event..");
                SP_ASSERT(callback != nullptr, "cannot have callback set to nullptr");
                auto* stream = findStream(type_id<E>(), createEventStream<E>);

                SP_ASSERT(stream != nullptr, "error with finding stream");
                stream->emplace_observer(name, std::forward<Callback<E>>(callback));
            }

            template<typename E>
            void createStreamExplicit()
            {
                findStream(type_id<E>(), createEventStream<E>);
            }
            template<typename E>
            void removeObserver(const uint32_t name)
            {
                static_assert(is_event_valid<E>(), "invalid event..");

                constexpr auto eventID = type_id<E>();
                auto* stream = findStream(eventID, createEventStream<E>);
                SP_ASSERT(stream != nullptr, "error with finding stream");

                stream->remove_observer(name);
            }

            template<typename E>
            void dispatch(E event)
            {
                static_assert(is_event_valid<E>(), "invalid event..");
                sp::Any any{std::move(event)};

                auto* stream = findStream(type_id<E>(), createEventStream<E>);
                SP_ASSERT(stream != nullptr, "error with finding stream");

                stream->dispatch(any);
            }

            void removeObserverAnywhere(const uint32_t name)
            {
                for(auto& eventStream : m_event_streams)
                {
                    eventStream->remove_observer(name);
                }
            }

            //notifies all observers..
            void process()
            {
                for(auto & eventStream : m_event_streams)
                {
                    eventStream->process();
                }
            }

            uint32_t genUniqueID()
            {
                return ++m_generator;
            }
    };
}

#endif // SUBJECT_H_INCLUDED
