#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H
#include <sp/math/mat.h>
#include <sp/math/vec.h>

namespace sp
{
    class Transformable
    {
        public:
                            Transformable();
            virtual        ~Transformable();

            void            setPosition(float x, float y);
            void            setPosition(const vec2f& pos);
            void            setRotation(float angle);
            void            setScale(float fx, float fy);
            void            setScale(const vec2f& scale);
            void            setOrigin(float x, float y);
            void            setOrigin(const vec2f& orig);

            const vec2f&    getPosition() const;
            const vec2f&    getScale() const;
            float           getRotation() const;
            const vec2f&    getOrigin() const;

            void            move(float x, float y);
            void            move(const vec2f& offset);

            void            rotate(float angle);
            void            scale(float x, float y);
            void            scale(const vec2f& scale);

            const mat&      getMatrix() const;
            const mat&      getInverseMatrix() const;

        private:
            vec2f           m_origin;
            vec2f           m_position;
            float           m_rotation;
            vec2f           m_scale;
            mutable mat     m_matrix;
            mutable bool    m_update_mat;
            mutable mat     m_inv_matrix;
            mutable bool    m_update_inv_mat;
    };
}

#endif // TRANSFORMABLE_H
