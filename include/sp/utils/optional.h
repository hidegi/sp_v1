#ifndef OPTIONAL_H_INCLUDED
#define OPTIONAL_H_INCLUDED
#include <sp/sp.h>
#include <memory>

namespace sp {
    template<typename T>
    class SP_API Optional{
        public:
            Optional() noexcept = default;
            Optional(const T& val) noexcept :
                m_ptr(std::make_unique<T>(val))
            {
            }

            Optional(T&& val) noexcept :
                m_ptr(std::make_unique<T>(std::move(val)))
            {
            }

            Optional(const Optional& other) :
                m_ptr(other.m_ptr ? std::make_unique<T>(*other.m_ptr) : nullptr)
            {
            }

            Optional(Optional&& other) noexcept = default;

            template<typename ... A>
            void emplace(A&& ... a)
            {
                m_ptr = std::make_unique<T>(a...);
            }

            void reset() noexcept {m_ptr = nullptr;}

            T& operator*() { return *m_ptr;}
            const T& operator*() const {return *m_ptr;}

            T* operator->() {return m_ptr.get();}
            const T* operator->() const { return m_ptr.get();}

            Optional& operator=(std::nullptr_t) noexcept {
                m_ptr = nullptr;
                return *this;
            }

            Optional& operator=(const T& val) noexcept
            {
                m_ptr(std::make_unique<T>(val));
                return *this;
            }

            Optional& operator=(T&& val) noexcept {
                m_ptr = std::make_unique<T>(std::move(val));
                return *this;
            }

            Optional& operator=(const Optional& other) noexcept {
                m_ptr = other.m_ptr ? std::make_unique<T>(other.m_ptr) : nullptr;
                return *this;
            }

            Optional& operator=(Optional&& other) noexcept = default;

            bool operator==(std::nullptr_t) noexcept
            {
                return (m_ptr == nullptr);
            }

            bool operator!=(std::nullptr_t) noexcept
            {
                return (m_ptr != nullptr);
            }

            explicit operator bool() const noexcept {
                return m_ptr != nullptr;
            }

            const T& value() const {
                return *m_ptr;
            }

            T& value() {
                return *m_ptr;
            }

        private:
            std::unique_ptr<T> m_ptr;
    };
}


#endif // OPTIONAL_H_INCLUDED
