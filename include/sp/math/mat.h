#ifndef SP_MAT_H
#define SP_MAT_H
#include <sp/sp.h>
#include <sp/math/math.h>
#include <sp/math/vec.h>
#include <sp/math/rect.h>

/**
 *	matrix class that can define operations,
 *	transforms, perspectives, mvps, etc..
 */
namespace sp
{
	extern const float		IDENTITY[16];

	/**
	 *	default matrix api for SP2D..
	 */

#ifndef SP_DEFINE_API_3D
	struct mat
	{
							mat();
			explicit		mat(	float s00, float s01, float s02,
									float s10, float s11, float s12,
									float s20, float s21, float s22);

							mat(const float[16]);
							mat(const mat& other);
							mat(mat&& other) noexcept;

			mat&			move(float x, float y);
			mat&			move(const vec2f& pos);

			rectf           transformRect(const rectf& area) const;
			mat&            rotate(float angle);
			mat&            rotate(float angle, float center_x, float center_y);
			mat&            rotate(float angle, const vec2f& c);
            mat&            scale(float x, float y);
            mat&            scale(const vec2f& scale);
            mat&            scale(float sx, float sy, float cx, float cy);
            mat&            scale(const vec2f& scale, const vec2f& center);



			vec2f			transformVertex(float x, float y) const;
			vec2f			transformVertex(const vec2f& v) const;

			float 			operator[](int n) const;

			mat&			operator=(const mat& other);
            mat&			operator=(mat&& other) noexcept;
			mat&			operator=(const float[16]);

			mat&            cat(const mat& m);
			const float*	operator()() const;
			const float*	operator!() const;

			void            debug_print() const;
		private:
			mutable float	matrix[16];
			mutable float	invmatrix[16];
			mutable bool	recalculate;
	};

    mat		operator* (const mat& m1, const mat& m2);
    mat&    operator*=(mat& m1, const mat& m2);
    vec2f   operator* (const mat& m, const vec2f& v);

#else

	struct mat4
	{
							mat4();
							mat4(float s);
							mat4(float m[16]);

							mat4(const mat3&);

				explicit	mat4(	float s00, float s01, float s02
									float s10, float s11, float s12
									float s20, float s21, float s22);

				explicit	mat4(	float s00, float s01, float s02, float s03,
									float s10, float s11, float s12, float s13
									float s20, float s21, float s22, float s23,
									float s30, float s31, float s32, float s33);


		mat4				operator*(const mat4& other) const;
		mat4&				operator*=(mat4& other);

		float 				operator[](int n) const;
		float				matrix[16];
	};

	struct mat<int r, int c>
	{
	}
#endif
}
#endif
