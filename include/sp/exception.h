#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED
#include <sp/sp.h>
#include <sp/string.h>
#include <exception>
#define SP_EXCEPTION(msg)   \
    Exception{String(L"SP exception: ") + msg + String(L"..")};

namespace sp
{
    class SP_API Exception : public std::runtime_error
    {
        public:
            Exception(const String& args) :
                std::runtime_error{args.toStdString()}
            {
                fprintf(stderr, args.toStdString().c_str());
            }
    };
}

#endif // EXCEPTION_H_INCLUDED
