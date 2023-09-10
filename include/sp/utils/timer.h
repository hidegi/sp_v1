#ifndef TIMER_H
#define TIMER_H

#include <sp/sp.h>
#include <sp/utils/Duration.h>
#include <sp/utils/Optional.h>

#include <functional>
#include <memory>
#include <vector>

namespace sp {


class Timer : public std::enable_shared_from_this<Timer>
{
    public:
        typedef std::shared_ptr<Timer> Ptr;

        ///no copy..
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;


        static std::shared_ptr<Timer> create(
                const std::function<void()>& callback,
                Duration interval,
                bool enable = true
                );



        static std::shared_ptr<Timer> create(
                const std::function<void(std::shared_ptr<Timer>)>& callback,
                Duration interval,
                bool enable = true
                );

        static void scheduleCallback(
                std::function<void()> callback,
                Duration interval = Duration()
                );

        void        setInterval(Duration interval);
        Duration    getInterval() const;

        void        setEnabled(bool enabled);
        bool        isEnabled() const;

        void        setCallback(const std::function<void()>& callback);
        void        setCallback(const std::function<void(std::shared_ptr<Timer>)>& callback);

        void        restart();

        static bool                 updateTime(Duration elapsedTime);
        static Optional<Duration>   getNextScheduledTime();
        static void                 clearTimers();

    protected:
        Timer() = default;

    private:
        static std::vector<std::shared_ptr<Timer>>  m_activeTimers;
        bool                                        m_repeats{false};
        bool                                        m_enabled{false};
        Duration                                    m_interval;
        std::function<void()>                       m_callback;
        Duration                                    m_remainingDuration;
};
}
#endif // TIMER_H
