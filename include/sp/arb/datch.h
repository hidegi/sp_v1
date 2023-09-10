#ifndef DATCH_H
#define DATCH_H
#include <functional>

namespace sp
{
    //a datch in particular is a data structure..
    //in this case, it is fused together into a data structure and a map..

    /*
    template<typename T>
    struct Node
    {
        public:
            Leaf*   parent  {nullptr};
            Leaf*   _1      {nullptr};
            Leaf*   _2      {nullptr};
            Leaf*   _3      {nullptr};
            Leaf*   _4      {nullptr};
            T       data;
    };
    */

    //map, client side..
    template<typename T>
    class Datch
    {
        public:
            //datch creates a texture for floor..
            //linear order..


            Datch();
           ~Datch();

            static Datch*       create();

            void                add(T t);
            T*                  find_if(std::function<bool(const T& t1)> cmp);
            void                setComparator(std::function<bool(const T& t1, const T& t2)> cmp);
            bool                remove_if(std::function<bool(const T& t1)> cmp);
            bool                remove(T t);
            void                clear();
            size_t              size();

        private:
            //Datch(size_t level, size_t max_level, Datch* p = nullptr);
            //some pseudo predicate..
            /*
             *  sort by:
             *  ((p1.x * p1.y) + p1.x) < ((p2.x * p2.y) + p2.x)
             */

            //bool        compare(const vec2f& p1, const vec2f& p2);

            //Texture*    texture;

            bool        isDatchelladeComplete() const;

            //Node<T>     m_crown;
            /*
            Datch*      m_super;
            Datch*      m_parent;
            Datch*      m_1;
            Datch*      m_2;
            Datch*      m_3;
            Datch*      m_4;
            */

            Datch*      datch;
            std::function<bool(const T& t1, const T& t2)>   comparator;

            size_t      m_size;
            size_t      m_level;
            size_t      m_max_level;
    };

    /*
    template<typename T>
    Datch<T>::Datch() :
        m_super     {this},
        m_parent    {nullptr},

        m_1         {nullptr},
        m_2         {nullptr},
        m_3         {nullptr},
        m_4         {nullptr},
        m_data      {nullptr},

        m_level     {0},
        m_max_level {0}
    {
    }

    template<typename T>
    Datch<T>::Datch(size_t level, size_t max_level, Datch* p) ;
        m_parent    {p},
        m_1         {nullptr},
        m_2         {nullptr},
        m_3         {nullptr},
        m_4         {nullptr},
        m_data      {nullptr},
        m_level     {level},
        m_max_level {max_level}
    {
        m_super = p->m_super;
    }
    */
}

#endif // DATCH_H
