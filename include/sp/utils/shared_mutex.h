#ifndef SP_SHARED_MUTEX_H_INCLUDED
#define SP_SHARED_MUTEX_H_INCLUDED
#include <sp/sp.h>
#include <sp/exception.h>
#include <array>
#include <atomic>
#include <thread>
#if SP_COMPILE_VERSION >= 17
    #include <shared_mutex>
#endif


#ifndef LEVEL1_DCACHE_LINESIZE
#define LEVEL1_DCACHE_LINESIZE 64
#endif

namespace sp
{
#if SP_COMPILE_VERSION >= 17
        using shared_mutex = std::shared_mutex;
#else
    //will have 4 entry locks..
    class shared_mutex
    {
        struct entry_lock
        {
            const static uint64_t WRITE_MASK = 0x8000000000000000;
            const static uint64_t READ_MASK  = ~WRITE_MASK;

            std::atomic<uint64_t>   wr_lock;
            entry_lock() : wr_lock(0)
            {
            }
        };
#ifdef SP_PLATFORM_WINDOWS
            __declspec(align(LEVEL1_DCACHE_LINESIZE));
#else
            __attribute__((aligned(LEVEL1_DCACHE_LINESIZE)));
#endif
            std::array<entry_lock, 4>   el_;
            static inline std::atomic<size_t>         idx_hint_{0};

            static uint64_t get_hint_idx()
            {
                while(true)
                {
                    size_t cur_hint = idx_hint_.load();
                    /**
                     *  weak compare in the sense that it returns false,
                     *  event if the value of the object is equal to @arg cur_hint,
                     *  causing a spurious failure on some platforms, where a
                     *  specific series of instructions is used to implement it..
                     *
                     *  comparison and copying are almost identical to memcmp, memcpy..
                     *  weak comparisons excel better in a loop..
                     */
                    if(idx_hint_.compare_exchange_weak(cur_hint, cur_hint + 1))
                        return cur_hint;
                }
            }

            //return id for current mutex..
            inline static size_t get_thread_idx()
            {
                const thread_local size_t rv = get_hint_idx() % 4;
                return rv;
            }

        public:
            shared_mutex()
            {
            }

            void lock_shared()
            {
                while(true)
                {
                    size_t cur_rw_lock = el_[get_thread_idx()].wr_lock.load();
                    if(entry_lock::WRITE_MASK & cur_rw_lock)
                    {
                        std::this_thread::yield();
                        continue;
                    }

                    if(el_[get_thread_idx()].wr_lock.compare_exchange_weak(cur_rw_lock, cur_rw_lock + 1))
                        break;
                }
            }

            void unlock_shared()
            {
                while(true)
                {
                    size_t cur_rw_lock = el_[get_thread_idx()].wr_lock.load();
#ifndef _RELEASE
                    if(entry_lock::WRITE_MASK & cur_rw_lock)
                        throw SP_EXCEPTION("fatality: cannot perform shared unlock since unlock shared but currently WRITE_MASK is locked");
#endif
                    if(el_[get_thread_idx()].wr_lock.compare_exchange_weak(cur_rw_lock, cur_rw_lock - 1))
                        break;
                }
            }

            void lock()
            {
                for(size_t i = 0; i < 4; ++i)
                {
                    while(true)
                    {
                        size_t cur_rw_lock = el_[i].wr_lock.load();
                        if(cur_rw_lock)
                        {
                            std::this_thread::yield();
                            continue;
                        }

                        if(el_[i].wr_lock.compare_exchange_weak(cur_rw_lock, entry_lock::WRITE_MASK))
                            break;
                    }
                }
            }

            void unlock()
            {
                for(size_t i = 0; i < 4; ++i)
                {
                    while(true)
                    {
                        size_t cur_rw_lock = el_[i].wr_lock.load();
#ifndef _RELEASE
                        if(cur_rw_lock != entry_lock::WRITE_MASK)
                            throw SP_EXCEPTION("fatality: cannot perform unlock, since this entry is shared locked or not initialized");
#endif
                        if(el_[i].wr_lock.compare_exchange_weak(cur_rw_lock, 0))
                            break;
                    }
                }
            }

            ~shared_mutex()
            {
            }
    };

    //std::atomic<size_t> shared_mutex::idx_hint_{0};

    class x_lock
    {
            shared_mutex&   sm_;
        public:
            x_lock(shared_mutex& sm) : sm_(sm)
            {
                sm_.lock();
            }

            ~x_lock()
            {
                sm_.unlock();
            }
    };

    class s_lock
    {
            shared_mutex& sm_;
        public:
            s_lock(shared_mutex& sm) : sm_(sm)
            {
                sm_.lock_shared();
            }

            ~s_lock()
            {
                sm_.unlock_shared();
            }
    };
#endif
}

#endif // SHARED_MUTEX_H_INCLUDED
