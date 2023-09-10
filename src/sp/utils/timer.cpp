#include <sp/utils/Timer.h>
#include <algorithm>

namespace sp {
    namespace {
        ///hidden clock.. o.o
        struct MakeSharedTimer : public Timer {};

        template<typename Fn>
        std::shared_ptr<Timer> createTimer(const Fn& callback, Duration interval, bool enable)
        {
            auto timer = std::make_shared<MakeSharedTimer>();
            timer->setInterval(interval);
            timer->setCallback(callback);

            if(enable)
                timer->setEnabled(true);
            return timer;
        }
    } 

    std::vector<std::shared_ptr<Timer>> Timer::m_activeTimers;

        /**
         *  this triggers a callback after a given time (duration)..
         *
         *  a parameterless callback is expected..
         *  an interval indicates a duration when the callback is supposed to be invoked..
         *  a flag tells if the callback is supposed to be immediately invoked..
         */
    std::shared_ptr<Timer> Timer::create(const std::function<void()>& callback, Duration interval, bool enable)
    {
        auto timer = createTimer(callback, interval, enable);
        timer->m_repeats = true;
        return timer;
    }
        /**
         *  this triggers a callback after a given time (duration)..
         *
         *  a callback is expected that takes in a timer as parameter..
         *  called at every interval when the timer is enabled..
         *
         *  an interval indicates a duration when the callback is supposed to be invoked..
         */
    std::shared_ptr<Timer> Timer::create(const std::function<void(std::shared_ptr<Timer>)>& callback, Duration interval, bool enable)
    {
        auto timer = createTimer(callback, interval, enable);
        timer->m_repeats = true;
        return timer;
    }

        /**
		 *	starts the timer immediately..
		 *
		 *	a callback is expected that is invoked, every time the timer is enabled..
		 *	an interval indicates a duration until a callback should occur; this parameter
		 *	is not intended to be used to trigger a callback at next update..
		 *
		 *	a scheduled call cannot be canceled..
		 *	this functionality is used for a proper timer that is then stopped after the callback..
		 */
    void Timer::scheduleCallback(std::function<void()> callback, Duration interval)
    {
        auto timer = createTimer(callback, interval, true);
        timer->m_repeats = false;
    }

    void Timer::setInterval(Duration interval)
    {
        if(m_interval == interval)
            return;

        m_interval = interval;
        if(m_enabled)
            restart();
    }

    Duration Timer::getInterval() const {return m_interval;}

    void Timer::setEnabled(bool enabled)
    {
        ///either push back or erase "this" from static vector..
        if(m_enabled != enabled)
        {

            if(enabled)
                /**
                 *  std::shared_ptr<T> shared_from_this();
                 *
                 *  returns an std::shared_ptr<T> that shared ownership of *this,
                 *  with all existing std::shared_ptr that refer to *this..
                 */
                m_activeTimers.push_back(shared_from_this());
            else
            {
                const auto it = std::find(m_activeTimers.begin(), m_activeTimers.end(), shared_from_this());
                if(it != m_activeTimers.end())
                    m_activeTimers.erase(it);
            }
        }
    }

    ///enabled getter..
    bool Timer::isEnabled() const {
        return m_enabled;
    }

    ///callback setter..
    void Timer::setCallback(const std::function<void()>& callback)
    {
        m_callback = callback;
    }

    ///callback setter..
    void Timer::setCallback(const std::function<void(std::shared_ptr<Timer>)>& callback)
    {
        //[=] by copy..
        m_callback = SP_LAMBDA_CAPTURE_EQ_THIS{callback(shared_from_this());};
    }

    void Timer::restart()
    {
        m_remainingDuration = m_interval;
    }

    bool Timer::updateTime(Duration elapsedTime)
    {
        if(m_activeTimers.empty())
            return false;

        bool timerTriggered = false;
        auto activeTimers = m_activeTimers; //copy..

        ///only pushed back if enabled; enable = push back..
        for(auto& timer : activeTimers)
        {
            if(timer->m_remainingDuration > elapsedTime)
               timer->m_remainingDuration -= elapsedTime;
            else {
                timerTriggered = true;
                timer->m_callback();    ///trigger line..
                timer->m_remainingDuration = timer->m_interval;

                if(!timer->m_repeats)
                    timer->setEnabled(false);
            }
        }

        return timerTriggered;
    }

    Optional<Duration> Timer::getNextScheduledTime()
    {
        if(m_activeTimers.empty())
            return Optional<Duration>();

        Duration minDuration = m_activeTimers[0]->m_remainingDuration;

        for(std::size_t i = 1; i < m_activeTimers.size(); ++i)
        {
            if(m_activeTimers[i]->m_remainingDuration < minDuration)
                minDuration = m_activeTimers[i]->m_remainingDuration;
        }

        return minDuration;
    }

    void Timer::clearTimers()
    {
        m_activeTimers.clear();
    }
}
