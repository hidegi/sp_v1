#include <sp/gxsp/vertex_array.h>
#include <sp/utils/helpers.h>
#include <algorithm>

namespace sp
{
	VertexArray::VertexArray(bool unique_only) :
		m_length{0},
		m_type	{0},
		m_unique_only{unique_only}
	{
	}

	VertexArray::VertexArray(SPint32 type, std::size_t vertexCount, bool unique_only) :
		m_type	        {type},
		m_length        {vertexCount},
		m_unique_only   {unique_only}
	{
	    m_vertices.reserve(vertexCount);
	}

	VertexArray::VertexArray(const VertexArray& other) :
		m_type		    {other.m_type},
		m_length	    {other.m_length},
		m_vertices	    {other.m_vertices},
		m_unique_only   {other.m_unique_only}
	{
	}


	VertexArray::VertexArray(VertexArray&& other) noexcept :
		m_type		    {std::move(other.m_type)},
		m_length	    {std::move(other.m_length)},
		m_vertices  	{std::move(other.m_vertices)},
		m_unique_only   {std::move(other.m_unique_only)}
	{
	}

	VertexArray& VertexArray::operator=(const VertexArray& other)
	{
		if(this != &other)
		{
			m_type 		= other.m_type;
			m_length 	= other.m_length;
			m_vertices	= other.m_vertices;
		}
		return *this;
	}

	VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
	{
		if(this != &other)
		{
			m_type 		= std::move(other.m_type);
			m_length 	= std::move(other.m_length);
			m_vertices	= std::move(other.m_vertices);
		}
		return *this;
	}

	std::size_t VertexArray::length() const {return m_length;}

	void VertexArray::add(const Vertex& vertex)
	{
	    if(m_unique_only && !m_vertices.empty())
        {
            for(std::size_t index = 0; index < m_vertices.size(); ++index)
            {
                Vertex v1 = m_vertices[index];
                if(
                   !spHelperCompareFloat(v1.position.x, vertex.position.x)
                && !spHelperCompareFloat(v1.position.y, vertex.position.y) )
                {
                    return;
                }
            }
        }
        m_vertices.push_back(vertex);
        m_length = m_vertices.size();

    }

	void VertexArray::resize(std::size_t count)
	{
		m_vertices.resize(count);
	}

	void VertexArray::reset()
	{
		m_vertices.clear();
	}

	void VertexArray::enableUniqueAddOnly(bool enable)
	{
	    m_unique_only = enable;
	}

	bool VertexArray::uniqueOnly() const {return m_unique_only;}

	const Vertex& VertexArray::operator[](int n) const
	{
		SP_ASSERT(n >= 0 && n < m_length, "cannot access element outside bounds");
		return m_vertices[n];
	}

	Vertex& VertexArray::operator[](int n)
	{
		SP_ASSERT(n >= 0 && n < m_length, "cannot access element outside bounds");
		return m_vertices[n];
	}

	SPint32 VertexArray::getType() const {return m_type;}
	void VertexArray::setType(SPint32 type) {m_type = type;}
	Vertex* VertexArray::getVertices() {return &m_vertices[0];}

    void VertexArray::setColor(sp::VertexArray& va, const sp::Color& color){
        for(size_t i = 0; i < va.length(); i++){
            va[i].color = color;
        }
    }

    void VertexArray::move(sp::VertexArray& va, const sp::vec2f& d){
        for(size_t i = 0; i < va.length(); i++){
            va[i].position += d;
        }
    }

    /*
    void VertexArray::darken(sp::VertexArray& va, float r){
        for(size_t i = 0; i < va.length(); i++){
            va[i].color = darken(va[i].color, r);
        }
    }*/

    /*
    void VertexArray::lighten(sp::VertexArray& va, float r){
        for(size_t i = 0; i < va.length(); i++){
            va[i].color = lighten(va[i].color, r);
        }
    }*/

    /*
    void VertexArray::interpolate(sp::VertexArray& va, const sp::Color& c, float r){
        for(size_t i = 0; i < va.length(); i++){
            va[i].color = interpolate(va[i].color, c, r);
        }
    }

    void VertexArray::complementary(sp::VertexArray& va){
        for(size_t i = 0; i < va.length(); i++){
            va[i].color = complementary(va[i].color);
        }
    }*/
}
