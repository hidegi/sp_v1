#ifndef OBSERVER_H_INCLUDED
#define OBSERVER_H_INCLUDED
#include <sp/sp.h>
#include <sp/event/type_id.h>
#include <sp/event/observer_rook.h>
namespace sp
{
    class SP_API Observer final
    {
        public:
            typedef std::shared_ptr<Observer>           Ptr;
            typedef std::shared_ptr<const Observer>     ConstPtr;
            explicit Observer() :
                m_subject   {nullptr},
                m_id        {0}
            {
            }

            explicit Observer(std::shared_ptr<Subject> sub) :
                m_subject {sub}
            {
                SP_ASSERT(m_subject != nullptr, "cannot have subject set to nullptr");
                m_id = m_subject->genUniqueID();
            }

            Observer(const Observer& other) :
                m_subject   {std::move(other.m_subject)}
            {
                SP_ASSERT(m_subject != nullptr, "cannot have subject set to nullptr");
                m_id = m_subject->genUniqueID();
            }

            Observer(const Observer&& other) :
                m_subject   {std::move(other.m_subject)}
            {
                SP_ASSERT(m_subject != nullptr, "cannot have subject set to nullptr");
                m_id = m_subject->genUniqueID();
            }

            Observer& operator=(const Observer& other)
            {
                if(this != &other)
                {
                    m_subject.reset();
                    m_subject = std::move(other.m_subject);
                }
                return *this;
            }

            Observer& operator=(Observer&& other)
            {
                if(this != &other)
                {
                    m_subject.reset();
                    m_subject = std::move(other.m_subject);
                }
                return *this;
            }

            void disconnectAll()
            {
                //ObserverRook::disconnectAll()
            }
           ~Observer()
            {
                if(m_subject)
                {
                    disconnectAll();
                }
            }

            template<typename E>
            void listen(Callback<E> callback)
            {
                //m_subject->addListener(m_id, std::move(callback));
                ObserverRook::connect<E>(m_subject, m_id, std::move(callback));
            }

        private:
            std::shared_ptr<Subject>    m_subject;
            uint32_t                    m_id;

    };
}


#endif // OBSERVER_H_INCLUDED
