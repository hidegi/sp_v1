#ifndef SP_H_INCLUDED
#define SP_H_INCLUDED

#if defined(_WIN32)
    #define SP_SYSTEM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #include "TargetConditionals.h" ///mac osx specific header..
    #if TARGET_OS_MAC
        #define SP_SYSTEM_MACOS
    #endif // TARGET_OS_MAC
#elif defined(__unix__)
    #define SP_SYSTEM_LINUX
#endif // defined

#ifndef SP_DYNAMIC
    #define SP_STATIC
	#define GLEW_STATIC
#endif // SFML_STATIC

#ifdef SP_RESOLVE_INCLUSION
	#define GLFW_INCLUDE_NONE
#endif

#ifndef SP_STATIC
    #ifdef SP_SYSTEM_WINDOWS
        #ifdef SP_EXPORT
            #define SP_API __declspec(dllexport)
        #else
            #define SP_API __declspec(dllexport)
        #endif // SP_EXPORT
    #else
        #define SP_ABI __attribute__((__visibility__("default")))
    #endif // SP_SYSTEM_WINDOWS
#else
	#define    SP_API
#endif // SP_STATIC

#if defined(WINAPI) && defined(SP_SYSTEM_WINDOWS)
    #undef  WINAPI
    #define WINAPI __stdcall
    #define SP_WINAPI_DEFINED
#endif // defined

#if defined(WINGDI) && defined(SP_SYSTEM_WINDOWS)
    #undef  WINGDI
    #define WINGDI __stdcall
    #define SP_WINGDI_DEFINED
#endif // defined

#if defined(CALLBACK) && defined(SP_SYSTEM_WINDOWS)
    #undef  CALLBACK
    #define CALLBACK __stdcall
    #define SP_CALLBACK_DEFINED
#endif // defined

#if defined(_MSC_VER)
    #define SP_MSC_VER _MSC_VER
#elif defined(__GNUC__)
    #define SP_GNUC __GNUC__
	#define SP_GNUC_MINOR __GNUC_MINOR__
	#define SP_GNUC_PATCHLEVEL __GNUC_PATCHLEVEL__
	#define SP_GCC_VERSION (SP_GNUC * 10000 + SP_GNUC_MINOR * 100 + SP_GNUC_PATCHLEVEL)
    #include <stdint.h>
#elif defined(__clang__)
    #define SP_CLANG __clang__
    #include <stdint.h>
#elif defined(__MINGW32__)
    #define SP_MINGW32 __MINGW32__
    #include <stdint.h>
#elif defined(__MINGW64__)
    #define SP_MINGW64 __MINGW64__
    #include <stdint.h>
#endif

#if defined(SP_MSC_VER)
	#define SP_UNREACHABLE __assume(0)
#elif defined(SP_GNUC) || defined(SP_CLANG)
	#define SP_UNREACHABLE __builtin_unreachable()
#else
	#define SP_UNREACHABLE
#endif

#if defined(SP_MSC_VER) && SP_MSC_VER >= 1800
	#define SP_VARARGS_SUPPORTED
#elif defined(SP_CLANG)
	#if __has_feature(cxx_variadic_templates)
		#define SP_VARARGS_SUPPORTED
	#endif
#elif defined(SP_GNUC) && (defined(__VARIADIC_TEMPLATES) || (SP_GNUC) > 4 || ((SP_GNUC) == 4 && (SP_GNUC_MINOR) >= 4 && defined(__GXX_EXPERIMENTAL_CXX0X__)))
	#define SP_VARARGS_SUPPORTED
#endif

#define SP_COMPILE_VERSION 14

#if SP_COMPILE_VERSION >= 20
    #include <version>
#endif // SP_COMPILE_VERSION

#if SP_COMPILE_VERSION >= 17
    #if (defined(__clang_major__) && (__clang_major__ >= 9) \
     || (defined(__GNUC__) && (__GNUC__ >= 9)) \
     || (defined(_MSC_VER) && (_MSC_VER >= 1914))
        #define SP_USE_STD_FILESYSTEM
    #endif
#endif // SP_COMPILE_VERSION
#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201304L)
    #define SP_CONSTEXPR    constexpr
#else
    #define SP_CONSTEXPR    inline
#endif // defined

#if __cplusplus >= 201703L
    #define SP_EMPLACE_BACK(obj, vec) auto& obj = vec.emplace_back();
#else
    #define SP_EMPLACE_BACK(obj, vec) vec.emplace_back(); auto& obj = vec.back();
#endif // __cplusplus

#ifndef SP_NO_DEPRECATED_WARNINGS
    #define SP_DEPRECATED(msg) [[deprecated(msg)]]
#else
    #define SP_DEPRECATED(msg)
#endif // SP_NO_DEPRECATED_WARNINGS

#if !defined(NDEBUG) && !defined(SP_NO_RUNTIME_WARNINGS)
    #include <iostream>
    #define SP_PRINT_WARNING(msg) {std::cerr << "SP warning: " << msg << "..\n";}
#else
    #define SP_PRINT_WARNING(msg)
#endif


#if !defined(NDEBUG) && !defined(SP_DISABLE_ASSERTS)
    #include <iostream>
    #include <cassert>
    #define SP_ASSERT(con, msg) {if(!(con)) {std::cerr << "SP assert: " << msg << "..\n"; assert(con);}}
#else
    #define SP_ASSERT(con, msg)
#endif

#if __cplusplus > 201703L
    #define SP_LAMBDA_CAPTURE_EQ_THIS [=,this]
#else
    #define SP_LAMBDA_CAPTURE_EQ_THIS [=]
#endif // __cplusplus

#define SP_PTR_SWAP(x, y)				\
	{									\
		void*	t;						\
		t = x;							\
		x = y;							\
		y = t;							\
	}

typedef char    SPchar;
typedef float   SPfloat;
typedef double  SPdouble;
typedef unsigned int SPuint;
typedef int          SPint;

typedef signed char     SPint8;
typedef unsigned char   SPuint8;

typedef signed short    SPint16;
typedef unsigned short  SPuint16;

typedef signed int      SPint32;
typedef unsigned int    SPuint32;

#if defined(SP_MSC_VER)
typedef signed __int64      SPint64;
typedef unsigned __int64    SPuint64;
#elif defined(SP_GNUC) || defined(SP_MINGW32) || \
      defined(SP_MINGW64) || defined(SP_CLANG)
typedef int64_t             SPint64;
typedef uint64_t            SPuint64;

#else
    typedef signed long long    SPint64;
    typedef unsigned long long  SPuint64;
#endif // defined

#if defined(SP_GNUC) || defined(SP_CLANG)
	///explicitly tells the compiler to ignore an unused variable (issues no warnings)..
	#define SP_REQUIRE_NO_ABSTRACT(T) typedef unsigned char sp_no_abstract[(sizeof(T))] __attribute__((unused))
#else
	#define SP_REQUIRE_NO_ABSTRACT(T) typedef unsigned char sp_no_abstract[(sizeof(T))]
#endif

#ifndef SP_DEFINE_NO_HELP_TRAITS
	#if __cplusplus >= 199711L
		#include <type_traits>

		#define IS_VOID(T)				std::is_void<T>::value
		#define IS_NULLPTR(T) 			std::is_null_pointer<T>::value
		#define IS_FLOATING(T)			std::is_floating_point<T>::value
		#define IS_ARRAY(T)				std::is_array<T>::value
		#define IS_ENUM(T)				std::is_enum<T>::value
		#define IS_UNION(T)				std::is_union<T>::value
		#define IS_FUNCTION(T)			std::is_function<T>::value
		#define IS_POINTER(T)			std::is_pointer<T>::value
		#define IS_LVAL_REF(T)			std::is_lvalue_reference<T>::value
		#define IS_RVAL_REF(T)			std::is_rvalue_reference<T>::value
		#define IS_MEM_OBJ_PTR(T)		std::is_member_object_pointer<T>::value
		#define IS_MEM_FN_PTR(T)		std::is_member_function_pointer<T>::value

		#define IS_FUNDAMENTAL(T)		std::is_fundamental<T>::value
		#define IS_ARITHMETIC(T)		std::is_arithmetic<T>::value
		#define IS_SCALAR(T)			std::is_scalar<T>::value
		#define IS_OBJECT(T)			std::is_object<T>::value
		#define IS_COMPOUND(T)			std::is_compound<T>::value
		#define IS_REFERENCE(T)			std::is_reference<T>::value
		#define IS_MEM_PTR(T)			std::is_member_pointer<T>::value

		#define IS_CONST(T)				std::is_const<T>::value

		#define IS_SAME(T, U)			std::is_same<T, U>::value
		#define IS_BASE_OF(T, U)		std::is_base_of<T, U>::value
		#define IS_CONVERTIBLE(T, U)	std::is_convertible<T, U>::value

		#define SP_ENABLE_IF(...) 				typename = typename std::enable_if_t<__VA_ARGS__>
		#define SP_ENABLE_IF_DEFAULT_NULL(...)	typename std::enable_if<__VA_ARGS__>::type* = nullptr
		#define SP_ENABLE_IF_ARITHMETIC(T) SP_ENABLE_IF((IS_ARITHMETIC(T)))
	#else
		#define SP_DEFINE_NO_HELP_TRAITS
	#endif
#endif

#ifndef SP_DEFINE_NO_CALLBACKS

typedef void(*SP_CALLBACK)();
typedef void(*SP_CALLBACK_PI)(SPint);
typedef void(*SP_CALLBACK_PUI)(SPuint);
typedef void(*SP_CALLBACK_PF)(SPfloat);
typedef void(*SP_CALLBACK_PD)(SPdouble);

typedef void(*SP_CALLBACK_PFv2)(SPfloat, SPfloat);
typedef void(*SP_CALLBACK_PDv2)(SPdouble, SPdouble);

typedef void(*SP_CALLBACK_PI8v2)(SPint8, SPint8);
typedef void(*SP_CALLBACK_PUI8v2)(SPuint8, SPuint8);

typedef void(*SP_CALLBACK_PU16v2)(SPuint16, SPuint16);
typedef void(*SP_CALLBACK_PUI16v2)(SPuint16, SPuint16);

typedef void(*SP_CALLBACK_PU32v2)(SPuint16, SPuint16);
typedef void(*SP_CALLBACK_PUI32v2)(SPuint16, SPuint16);

typedef void(*SP_CALLBACK_PU64v2)(SPuint16, SPuint16);
typedef void(*SP_CALLBACK_PUI64v2)(SPuint16, SPuint16);


typedef void(*SP_CALLBACK_PIv2)(SPint, SPint);
typedef void(*SP_CALLBACK_PUIv2)(SPuint, SPuint);
typedef void(*SP_CALLBACK_PDv2)(SPdouble, SPdouble);

typedef void(*SP_CALLBACK_PIv4)(SPint, SPint, SPint, SPint);

typedef void(*SP_CALLBACK_PCVPTR)(const void*);
typedef void(*SP_CALLBACK_PVPTR)(void*);
typedef void(*SP_CALLBACK_PCVVPTR)(const volatile void*);
#endif

enum SP_Usage {Static = 0x88E4, Dynamic = 0x88e8, Stream = 0x88E0};
enum SP_PrimitiveType : SPint32 {
	Points = 0x0000,
	Lines,
	LineLoop,
	LineStrip,
	Triangles,
	TriangleStrip,
	TriangleFan,
	Quads
};

#ifdef __cplusplus
namespace sp
{

#define  SP_TYPE_SWAP(T)				\
	template<typename T>				\
	inline void sp_swap(T& L, T& R)		\
	{									\
		using namespace std;			\
		std::swap(L, R);				\
	}
}
#endif

#define SP_IGNORE      -1
#define SP_FALSE       0
#define SP_TRUE        1

#define SP_BACKEND_GLFW
#ifdef SP_BACKEND_GLFW

typedef void(*SP_Int2)(int, int);
typedef void(*SP_Fn)(int, int);
typedef void(*SP_Size_Fn)(int, int);
typedef void(*SP_Size_Fn)(int, int);


enum class SP_Profile : int
{
	Core 		= 0,
	Compatible	= 0x32001,
	Any			= Compatible + 1
};

enum class SP_Client : int
{
	None		= 0,
	OpenGL		= 0x30001,
	OpenGLES	= OpenGL + 1,
};

enum class SP_ContextCreation : int
{
	Native		= 0x36001,
	Egl,
	Osmesa
};

enum class SP_Robustness : int
{
	None				= 0,
	NoResetNotification	= 0x31001,
	LoseContextOnReset
};

enum class SP_Release : int
{
	Any		= 0,
	Flush	= 0x35001,
	None
};

struct SP_Framebuffer
{
	int	redBits			{SP_IGNORE};
	int	greenBits		{SP_IGNORE};
	int	blueBits		{SP_IGNORE};
	int	alphaBits		{SP_IGNORE};
	int	stencilBits		{SP_IGNORE};
	int	depthBits		{SP_IGNORE};
	int	accumRedBits	{SP_IGNORE};
	int	accumGreenBits	{SP_IGNORE};
	int	accumBlueBits	{SP_IGNORE};
	int	accumAlphaBits	{SP_IGNORE};
	int auxBuffers		{SP_IGNORE};

	int stereo			{SP_FALSE};
	int	multisampling	{SP_FALSE};
	int	saturateRGB		{SP_FALSE};
};

struct SP_Config
{
	unsigned int 	width {800};
	unsigned int 	height{600};
	const char*		title = "window";

	SP_Profile			profile				{SP_Profile::Any};
	SP_Client			client				{SP_Client::OpenGL};
	SP_Robustness		robustness			{SP_Robustness::None};
	SP_Framebuffer		framebuffer			{};
	SP_Release			release				{SP_Release::Any};
	SP_ContextCreation	contextCreation		{SP_ContextCreation::Native};

	unsigned int		contextMajor		{3};
	unsigned int		contextMinor		{0};


	int					monitorCount		{0};		//do not use..
	int					refreshRate			{SP_IGNORE};
	int					forwardCompatible	{SP_FALSE};

	int					resizable			{SP_TRUE};
	int					visible				{SP_TRUE};
	int					decorated			{SP_TRUE};
	int					fullscreen			{SP_FALSE};
	int					focused				{SP_TRUE};
	int					iconify				{SP_FALSE};
	int					floating			{SP_FALSE};
	int					maximized			{SP_FALSE};
	int					centerCursor		{SP_FALSE};
	int					transparent			{SP_FALSE};
	int					focusOnShow			{SP_TRUE};
	int					scaleToMonitor		{SP_FALSE};
	int					debugContext		{SP_FALSE};
	int					doubleBuffer		{SP_TRUE};
};

/**
 *	both clang and the gnu compiler collection implement a non-standard extension to the C language to support RAII: the "cleanup" variable attribute..
 *	the following annotates a variable with a given destructor function that it will call when the variable goes out of scope..
 *
 *	void example_usage()
 *	{
 *		__attribute__((cleanup(fclosep))) FILE *logfile = fopen("logfile.txt", "w+");
 *		fputs("hello logfile!", logfile);
 *	}
 */
#define	SP_PROPERTY(E, V)	glfwWindowHint(E, V)
#endif
#endif // INTERFACE_H_INCLUDED
