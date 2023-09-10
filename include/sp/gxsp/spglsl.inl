void spCopyVec3(const sp::Color& src, vec3<float>& dst);
void spCopyVec3(const sp::Color& src, vec3<int>& dst);

void spCopyVec4(const sp::Color& src, vec4<float>& dst);
void spCopyVec4(const sp::Color& src, vec4<int>& dst);

void spCopyMat(const mat& src, _mat<3, 3>& dst);
void spCopyMat(const mat& src, _mat<4, 4>& dst);
void spCopyMat(const float* src, size_t length, float* dst);

template<size_t c, size_t r>
struct _mat
{
    explicit _mat(const float* ptr)
    {
        spCopyMat(ptr, c * r, pointer);
    }

    _mat(const mat& m)
    {
        spCopyMat(m, *this);
    }
    float pointer[c * r];
};

template<typename T>
struct vec3
{
    vec3() :
        x{0},
        y{0},
        z{0}
    {
    }

    vec3(T _x, T _y, T _z) :
        x{_x},
        y{_y},
        z{_z}
    {
    }

    template<typename U>
    explicit vec3(const vec3<U>& v) :
        x{static_cast<T>(v.x)},
        y{static_cast<T>(v.y)},
        z{static_cast<T>(v.z)}
    {
    }

    vec3(const Color& color)
    {
        spCopyVec3(color, *this);
    }

    T x, y, z;
};

template<typename T>
struct vec4
{
    vec4() :
        x{0},
        y{0},
        z{0},
        w{0}
    {
    }

    vec4(T _x, T _y, T _z, T _w) :
        x{_x},
        y{_y},
        z{_z},
        w{_w}
    {
    }

    template<typename U>
    explicit vec4(const vec4<U>& v) :
        x{static_cast<T>(v.x)},
        y{static_cast<T>(v.y)},
        z{static_cast<T>(v.z)},
        w{static_cast<T>(v.w)}
    {
    }

    vec4(const Color& color)
    {
        spCopyVec4(color, *this);
    }

    T x, y, z, w;
};
