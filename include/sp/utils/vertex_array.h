#ifndef SP_VERTEX_ARRAY_H
#define SP_VERTEX_ARRAY_H
#include <sp/utils/vertex.h>
#include <vector>
#include <memory>

namespace sp
{
	class VertexArray
	{
		public:
									VertexArray(bool unique_only = false);
			explicit 				VertexArray(SPint32 type, std::size_t length = 0, bool unique_only = false);

									VertexArray(const VertexArray&);
									VertexArray(VertexArray&&) noexcept;

			VertexArray&			operator=(const VertexArray&);
			VertexArray&			operator=(VertexArray&&) noexcept;

			std::size_t				length() const;
			void					add(const Vertex& vertex);
			void					resize(std::size_t length);
			void					reset();


			void                    enableUniqueAddOnly(bool enable);
            bool                    uniqueOnly() const;

				  Vertex&			operator[](int n);
			const Vertex&			operator[](int n) const;

			void					setType(SPint32 type);
			SPint32					getType() const;

			Vertex*					getVertices();

		private:
			std::size_t				m_length;
			std::vector<Vertex>		m_vertices;
			SPint32					m_type;
			bool                    m_unique_only;
	};
}
#endif
