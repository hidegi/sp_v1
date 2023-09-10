#ifndef MOTEX_PERKS_H
#define MOTEX_PERKS_H
#include <sp/exsp/event_bus.h>
#include <condition_variable>
#include <vector>
#include <memory>
enum class Flag : int
{
    NoOperation,
    PostponeCancel,
    PostponeContinue
};

class Perk
{
    public:
        virtual ~Perk() = default;
};

class AnyPassPerk : public Perk
{
    public:
        AnyPassPerk(std::shared_ptr<sp::EventBus> passTo) :
            _passToBus(std::move(passTo))
        {
        }

        Flag onPrePostponeEvent(sp::bus::PostponeHelper& postponeCall);

    private:
        std::shared_ptr<sp::bus::Bus>   _passToBus;
};

class PostponeBus
{
    public:
        PostponeBus(std::shared_ptr<sp::bus::Bus> hideBus) :
            _hideBus{std::move(hideBus)}
        {
        }

        template<typename Event>
        SP_CONSTEXPR bool postpone(Event event)
        {
            assert(_hideBus);
            return _hideBus->postpone(event);
        }

    private:
        std::shared_ptr<sp::bus::Bus>   _hideBus;
};

class PerkEventBus : public sp::EventBus
{
    public:
        class RegisterHelper
        {
            friend class PerkEventBus;
            public:
                template<typename Perk>
                RegisterHelper& registerPrePostpone(Flag (Perk::*method)(sp::bus::PostponeHelper&))
                {
                    _bus->_onPrePostpone.push_back(
                    std::bind(method, static_cast<Perk*>(_perk), std::placeholders::_1));
                    return *this;
                }

                template<typename Perk>
                RegisterHelper& registerPostPostpone(Flag (Perk::*method)(sp::bus::PostponeHelper&))
                {
                    _bus->_onPostPostpone.push_back(
                    std::bind(method, static_cast<Perk*>(_perk), std::placeholders::_1));
                    return *this;
                }

            private:
                PerkEventBus*   _bus;
                Perk*           _perk;

                RegisterHelper(PerkEventBus* bus, Perk* perk) :
                    _bus    (bus),
                    _perk   {perk}
                {
                }
        };
        RegisterHelper addPerk(std::shared_ptr<Perk> perk);

        template<typename T>
        T* getPerk()
        {
            auto found = std::find_if(_perks.begin(), _perks.end(), [](const std::shared_ptr<Perk>& perk)
            {
               return dynamic_cast<T*>(perk.get()) != nullptr;
            });
            if(found != _perks.end())
            {
                return static_cast<T*>(found->get());
            }
            return nullptr;
        }

    protected:
        bool postponeEvent(sp::bus::PostponeHelper& postponeCall) override;
    private:
        std::vector<std::shared_ptr<Perk>>                  _perks;
        std::vector<std::function<Flag(sp::bus::PostponeHelper&)>>   _onPrePostpone;
        std::vector<std::function<Flag(sp::bus::PostponeHelper&)>>   _onPostPostpone;

};

class WaitPerk : public Perk
{
    public:
        bool wait();
        bool waitFor(std::chrono::milliseconds timeout);
        Flag onPostponeEvent(sp::bus::PostponeHelper& postponeCall);

    private:
        std::condition_variable _eventWaiting;
        std::mutex              _waitMutex;
        bool                    _hasEvents = false;
};

class TagPerk : public Perk
{
    public:
        TagPerk(std::string tag, sp::bus::Bus* owner) :
            _tag        (std::move(tag)),
            _ownerBus   {owner}
        {
        }

        Flag onPrePostponeEvent(sp::bus::PostponeHelper& postponeCall);

        template<typename TagEvent>
        TagPerk& wrapTag()
        {
            static_assert(sp::inter::is_event_valid<TagEvent>(), "invalid tag event..");
            static_assert(sp::inter::is_event_valid<typename TagEvent::Event>(), "invalid tag event..");
            SP_CONSTEXPR auto eventID = sp::inter::type_id<typename TagEvent::Event>();
            _eventsToWrap[eventID] = [this](sp::Any event)
            {
                TagEvent newEvent{_tag, std::move(sp::any_cast<typename TagEvent::Event>(event))};
                _ownerBus->postpone<TagEvent>(std::move(newEvent));
            };

            return *this;
        }

    private:
        std::map<sp::inter::type_id_t, std::function<void(sp::Any)>>   _eventsToWrap;
        std::string         _tag;
        sp::bus::Bus*       _ownerBus;
};

#endif // PERKS_H
