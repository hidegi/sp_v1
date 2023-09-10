#ifndef MEM_WRAPPER_H_INCLUDED
#define MEM_WRAPPER_H_INCLUDED
#include <sp/sp.h>

namespace sp
{
    class SlotBase;

    class ClassMemberAbstract
    {

    };

    class ClassMethodAbstract
    {
        public:
            virtual ~ClassWrapperAbstract() {}
            virtual bool operator==(const ClassWrapperAbstract&) = 0;
            virtual bool operator!=(const ClassWrapperAbstract&) = 0;
    };
}


#endif // MEM_WRAPPER_H_INCLUDED
