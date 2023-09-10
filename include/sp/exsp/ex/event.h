#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

namespace exsp
{
    struct GoldUpdate
    {
        int goldCount;
    };

    struct NewTeamMember
    {
        std::string memberName;
    };

    template<typename T>
    struct TagEvent
    {
        using Event = T;
        std::string tag;
        Event data;
    };
}


#endif // EVENT_H_INCLUDED
