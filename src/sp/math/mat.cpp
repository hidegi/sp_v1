#include <sp/math/mat.h>
#include <sp/utils/helpers.h>
#include <memory.h>
#include <cmath>

namespace sp
{
	const float IDENTITY[16] =
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};

	mat::mat() :
		recalculate{true}
	{
		memcpy(matrix, IDENTITY, sizeof(float) * 16);
	}

	mat::mat(const float array[16]) :
		recalculate{true}
	{
		memcpy(matrix, array, sizeof(float) * 16);
	}
	mat::mat(	float s00, float s01, float s02,
				float s10, float s11, float s12,
				float s20, float s21, float s22) :
		recalculate{true}
	{
		matrix[0] = s00; matrix[4] = s01; matrix[8]  = 0.f; matrix[12] = s02;
		matrix[1] = s10; matrix[5] = s11; matrix[9]  = 0.f; matrix[13] = s12;
		matrix[2] = 0.f; matrix[6] = 0.f; matrix[10] = 1.f; matrix[14] = 0.f;
		matrix[3] = s20; matrix[7] = s21; matrix[11] = 0.f; matrix[15] = s22;
	}

	mat& mat::operator=(const float array[16])
	{
	    printf("copy operation array..\n");
		memcpy(matrix, array, sizeof(float) * 16);
		return *this;
	}

	mat::mat(const mat& other)
	{
		memcpy(matrix, other.matrix, sizeof(float) * 16);
		//mems(other.invmatrix, 0, sizeof(float) * 16);
		recalculate = true;
	}

	mat::mat(mat&& other) noexcept
	{
		memmove(matrix, other.matrix, sizeof(float) * 16);
		memset(other.invmatrix, 0, sizeof(float) * 16);
		recalculate = true;
	}


	mat& mat::operator=(const mat& other)
	{
		if(this != &other)
		{
			memcpy(matrix, other.matrix, sizeof(float) * 16);
			recalculate = true;
		}
		return *this;
	}

	mat& mat::operator=(mat&& other) noexcept
	{
		if(this != &other)
		{
			memmove(matrix, other.matrix, sizeof(float) * 16);
			recalculate = true;
		}
		return *this;
	}

	mat& mat::move(float x, float y)
	{
		mat m = mat{1.f, 0.f, x,
					0.f, 1.f, y,
					0.f, 0.f, 1.f};

		*this *= m;
		recalculate = true;
		return *this;
	}

	mat& mat::move(const vec2f& pos)
	{
		return move(pos.x, pos.y);
	}

	rectf mat::transformRect(const rectf& area) const
	{
	    const vec2f points[] =
	    {
            transformVertex(area.left, area.top),
            transformVertex(area.left, area.top + area.height),
            transformVertex(area.left + area.width, area.top),
            transformVertex(area.left + area.width, area.top + area.top)
	    };

	    float left      = points[0].x;
	    float top       = points[0].y;
	    float right     = points[0].x;
	    float bottom    = points[0].y;

	    for(int i = 1; i < 4; ++i)
        {
            if      (points[i].x < left)  left   = points[i].x;
            else if (points[i].x > right) right  = points[i].x;
            if      (points[i].y < top)   top    = points[i].y;
            if      (points[i].y > bottom)bottom = points[i].y;
        }

        return rectf{left, top, right - left, bottom - top};
	}

	mat& mat::rotate(float angle)
	{
	    float rad = angle * 3.141592654f / 180.f;
	    float cos = std::cos(rad);
	    float sin = std::sin(rad);

	    mat rotation(cos, -sin, 0,
                     sin,  cos, 0,
                       0,    0, 1);
        return cat(rotation);
	}

	mat& mat::rotate(float angle, float cx, float cy)
	{
	    float rad = angle * 3.141592654f / 180.f;
        float cos = std::cos(rad);
        float sin = std::cos(sin);

        mat rotation(cos, -sin, cx * (1 - cos) + cy * sin,
                     sin,  cos, cy * (1 - cos) - cx * sin,
                     0, 0, 1);

        return cat(rotation);
	}

	mat& mat::rotate(float angle, const vec2f& c)
	{
        return rotate(angle, c.x, c.y);
	}

	mat& mat::scale(float x, float y)
	{
        mat scale(x, 0, 0,
                  0, y, 0,
                  0, 0, 1);
        return cat(scale);
	}

	mat& mat::scale(const vec2f& s)
	{
	    return scale(s.x, s.y);
	}

    mat& mat::scale(float sx, float sy, float cx, float cy)
    {
        mat scale(sx, 0, cx * (1 - sx),
                  0, sy, cy * (1 - sy),
                  0, 0, 1);

        return cat(scale);
    }

    mat& mat::scale(const vec2f& s, const vec2f& c)
    {
        return scale(s.x, s.y, c.x, c.y);
    }

	mat& mat::cat(const mat& m)
	{
        const float* a = this->matrix;
		const float* b = m.matrix;
		*this =
		mat{
			a[0] * b[0]  + a[4] * b[1]  + a[12] * b[3],
            a[0] * b[4]  + a[4] * b[5]  + a[12] * b[7],
            a[0] * b[12] + a[4] * b[13] + a[12] * b[15],
			a[1] * b[0]  + a[5] * b[1]  + a[13] * b[3],
			a[1] * b[4]  + a[5] * b[5]  + a[13] * b[7],
			a[1] * b[12] + a[5] * b[13] + a[13] * b[15],
			a[3] * b[0]  + a[7] * b[1]  + a[15] * b[3],
			a[3] * b[4]  + a[7] * b[5]  + a[15] * b[7],
			a[3] * b[12] + a[7] * b[13] + a[15] * b[15]
		};

		return *this;
	}

	float mat::operator[](int n) const
	{
		SP_ASSERT(n > 0 && n < 16, "cannot access element outside bounds");
		return matrix[n];
	}
	const float* mat::operator()() const
	{
		return matrix;
	}

	vec2f mat::transformVertex(float x, float y) const
	{
		return
		vec2f
		{
			matrix[0] * x + matrix[4] * y + matrix[12],
			matrix[1] * x + matrix[5] * y + matrix[13]
		};
	}

	vec2f mat::transformVertex(const vec2f& v) const
	{
		return transformVertex(v.x, v.y);
	}

	void mat::debug_print() const
	{
	    for(int i = 0; i < 16; i++)
        {
            if(!((i) % 4))
                printf("\n");

            printf("%f ", matrix[i]);
        }
        printf("\n");
	}
	const float* mat::operator!() const
	{
        float det = matrix[0] * (matrix[15] * matrix[5] - matrix[7] * matrix[13]) -
                    matrix[1] * (matrix[15] * matrix[4] - matrix[7] * matrix[12]) +
                    matrix[3] * (matrix[13] * matrix[4] - matrix[5] * matrix[12]);

        if (spHelperCompareFloat(det, 0.f))
        {
            float m[16] =
						{(matrix[15] * matrix[5] - matrix[7] * matrix[13]) / det, -(matrix[15] * matrix[4] - matrix[7] * matrix[12]) / det, 0.f, (matrix[13] * matrix[4] - matrix[5] * matrix[12]) / det,
                        -(matrix[15] * matrix[1] - matrix[3] * matrix[13]) / det, (matrix[15] * matrix[0] - matrix[3] * matrix[12]) / det, 0.f, -(matrix[13] * matrix[0] - matrix[1] * matrix[12]) / det,
						  0.f, 0.f, 1.f, 0.f,
                         (matrix[7]  * matrix[1] - matrix[3] * matrix[5])  / det, -(matrix[7]  * matrix[0] - matrix[3] * matrix[4])  / det, 0.f, (matrix[5]  * matrix[0] - matrix[1] * matrix[4])  / det};


            memmove(invmatrix, m, sizeof(float ) * 16);
            return invmatrix;
        }
        return IDENTITY;
	}

	mat  operator* (const mat& m1, const mat& m2)
	{
	    return mat(m1).cat(m2);
	}

	mat& operator*=(mat& m1, const mat& m2)
	{
		return m1.cat(m2);
	}

	vec2f operator*(const mat& m, const vec2f& v)
	{
        return m.transformVertex(v);
	}

}
