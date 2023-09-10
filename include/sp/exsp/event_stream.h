#ifndef EVENT_STREAM_H_INCLUDED
#define EVENT_STREAM_H_INCLUDED
#include <sp/exception.h>
#include <sp/utils/any.h>

namespace sp::bus::stream
{
    class EventStream
    {
        public:
            virtual ~EventStream() = default;
            virtual void postpone(sp::Any event) = 0;
            virtual size_t process(size_t limit) = 0;
            virtual bool addListener(uint32_t listenerID, sp::Any callback) = 0;
            virtual bool removeListener(uint32_t listenerID) = 0;
    };

    //null-stream..
    class NoOpEventStream: public EventStream
    {
        public:
            virtual void postpone(sp::Any) override
            {
                throw SP_EXCEPTION("no operation");
            }

            virtual size_t process(size_t) override
            {
                throw SP_EXCEPTION("no operation");
            }

            virtual bool addListener(uint32_t, sp::Any) override
            {
                throw SP_EXCEPTION("no operation");
            }

            virtual bool removeListener(uint32_t) override
            {
                throw SP_EXCEPTION("no operation");
            }
    };
}

#endif // EVENT_STREAM_H_INCLUDED
