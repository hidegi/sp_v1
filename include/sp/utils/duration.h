#ifndef DURATION_H_INCLUDED
#define DURATION_H_INCLUDED

#include <sp/sp.h>
#include <cstdint>
#include <chrono>
#include <memory>

namespace sp {
    class SP_API Duration {
        /// operators: +,  - , / , * , %
        ///            +=, -=, /=, *=, %=
        public:
            SP_CONSTEXPR Duration() : m_duration{std::chrono::nanoseconds::zero()}
            {
            }

            template<typename R, typename P>
            SP_CONSTEXPR Duration(std::chrono::duration<R, P> duration) :
                m_duration{std::chrono::duration_cast<std::chrono::nanoseconds>(duration)}
            {
            }

            SP_CONSTEXPR Duration(int m) :
                Duration{std::chrono::milliseconds(m)}
            {
            }

            SP_CONSTEXPR float asSeconds() const {
                return static_cast<float>(static_cast<double>(m_duration.count()) / 1000000000.0);
            }

            SP_CONSTEXPR operator std::chrono::nanoseconds() const {return m_duration;}

            template<typename R, typename P>
            SP_CONSTEXPR operator std::chrono::duration<R, P>() const
            {
                return std::chrono::duration_cast<std::chrono::duration<R, P>>(m_duration);
            }

        private:
            std::chrono::nanoseconds        m_duration; ///internal duration in ns..
    };

    SP_API SP_CONSTEXPR bool operator==(const Duration& L, const Duration& R)
    {
        return std::chrono::nanoseconds(L) == std::chrono::nanoseconds(R);
    }

    SP_API SP_CONSTEXPR bool operator!=(const Duration& L, const Duration& R)
    {
        return !(L == R);
    }

    SP_API SP_CONSTEXPR bool operator>(const Duration& L, const Duration& R)
    {
        return std::chrono::nanoseconds(L) > std::chrono::nanoseconds(R);
    }

    SP_API SP_CONSTEXPR bool operator>=(const Duration& L, const Duration& R)
    {
        return std::chrono::nanoseconds(L) >= std::chrono::nanoseconds(R);
    }

    SP_API SP_CONSTEXPR bool operator<(const Duration& L, const Duration& R)
    {
        return std::chrono::nanoseconds(L) < std::chrono::nanoseconds(R);
    }

    SP_API SP_CONSTEXPR bool operator<=(const Duration& L, const Duration& R)
    {
        return std::chrono::nanoseconds(L) <= std::chrono::nanoseconds(R);
    }

    SP_API SP_CONSTEXPR Duration operator+(const Duration& L, const Duration& R)
    {
        return {std::chrono::nanoseconds(L) + std::chrono::nanoseconds(R)};
    }

    SP_API SP_CONSTEXPR Duration operator-(const Duration& L, const Duration& R)
    {
        return {std::chrono::nanoseconds(L) - std::chrono::nanoseconds(R)};
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration operator*(T L, const Duration& R)
    {
        return {L * std::chrono::nanoseconds(R)};
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration operator/(const Duration& L, T R)
    {
        return {std::chrono::nanoseconds(L) / R};
    }

    SP_API SP_CONSTEXPR float operator/(const Duration& L, const Duration& R)
    {
        return L.asSeconds() / R.asSeconds();
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration operator%(const Duration& L, T R)
    {
        return {std::chrono::nanoseconds(L) % R};
    }

    SP_API SP_CONSTEXPR Duration operator%(const Duration& L, const Duration& R)
    {
        return {std::chrono::nanoseconds(L) % std::chrono::nanoseconds(R)};
    }

    SP_API SP_CONSTEXPR Duration& operator+=(Duration& L, const Duration& R)
    {
        return L = L + R;
    }

    SP_API SP_CONSTEXPR Duration& operator-=(Duration& L, const Duration& R)
    {
        return L = L - R;
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration& operator*=(Duration& L, T R)
    {
        return L = L * R;
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration& operator/=(Duration& L, T R)
    {
        return L = L / R;
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::value>
    SP_API SP_CONSTEXPR Duration& operator%=(Duration& L, T R)
    {
        return L = L % R;
    }

    SP_API SP_CONSTEXPR Duration& operator%=(Duration& L, const Duration& R)
    {
        return L = L % R;
    }
}

#endif // DURATION_H_INCLUDED
