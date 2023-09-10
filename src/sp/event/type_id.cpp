#include <functional>
#include <sp/event/type_id.h>

namespace sp::detail
{
    type_id_t hash(const char* str)
    {
        uint32_t h = 5381;
        int c = 0;
        while(c = *str++)
        {
            h = ((h << 5) + h) + c;
        }

        return static_cast<type_id_t>(h);
    }
}
