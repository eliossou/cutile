#ifndef CUT_BASE
    #define CUT_BASE

    // Keyword used for compiled symbols that must not be referenced outside their own compilation unit.
    #define cut_internal static

    // Keyword used for functions variables that need to keep their value between calls.
    #define cut_persist static

    #define cut_thread_local _Thread_local

    typedef char            cut_s8;
    typedef unsigned char   cut_u8;

    typedef short           cut_s16;
    typedef unsigned short  cut_u16;

    #if defined(__clang__) || defined(__GNUC__)
        typedef __INT32_TYPE__          cut_s32;
        typedef unsigned __INT32_TYPE__ cut_u32;

        typedef __INT64_TYPE__          cut_s64;
        typedef unsigned __INT64_TYPE__ cut_u64;
    #elif defined(_MSC_VER)
        typedef __int32          cut_s32;
        typedef unsigned __int32 cut_u32;

        typedef __int64          cut_s64;
        typedef unsigned __int64 cut_u64;
    #endif

    typedef float  cut_f32;
    typedef double cut_f64;

    #if !defined(CUT_TARGET_PLATFORM_POINTER_SIZE)
        // Microsoft Visual C
        #if defined(_M_AMD64) || defined(_M_ARM64)
            #define CUT_TARGET_PLATFORM_POINTER_SIZE 8
        #elif defined(_M_IX86)
            #define CUT_TARGET_PLATFORM_POINTER_SIZE 4

        // GNU C Compiler
        #elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
            #define CUT_TARGET_PLATFORM_POINTER_SIZE 8
        #elif defined(__i386) || defined(__i386__)
            #define CUT_TARGET_PLATFORM_POINTER_SIZE 4

        #else
            cut_ct_assert(0, "Platform pointer size is not defined and could not be deduced.")
        #endif
    #endif

    #if CUT_TARGET_PLATFORM_POINTER_SIZE == 8
        typedef cut_u64 cut_uptrsize;
        typedef cut_s64 cut_sptrsize;
    #elif CUT_TARGET_PLATFORM_POINTER_SIZE == 4
        typedef cut_u32 cut_uptrsize;
        typedef cut_s32 cut_sptrsize;
    #endif

    #define cut_field_offset(StructureType, FieldName) (cut_uptrsize)(&(((StructureType *)0)->FieldName))
    #define cut_field_size(StructType, FieldName) sizeof(((StructType *)0)->FieldName)

    #if defined(__clang__) || defined(__GNUC__)
        #define cut_packed(Declaration) Declaration __attribute__((__packed__))
    #elif defined(_MSC_VER)
        #define cut_packed(Declaration) __pragma(pack(push, 1)) Declaration __pragma(pack(pop))
    #endif

    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
        #define cut_alignas(PositivePowOf2OrType) alignas(PositivePowOf2OrType)
    #else
        #define cut_alignas(PositivePowOf2OrType) _Alignas(PositivePowOf2OrType)
    #endif

    // Compiler alignas.
    #if defined(__clang__) || defined(__GNUC__)
        #define cut_cc_alignas(PositivePowOf2) __atribute__((aligned(PositivePowOf2)))
    #elif defined(_MSC_VER)
        #define cut_cc_alignas(PositivePowOf2) __declspec(align(PositivePowOf2))
    #endif

    #if defined(_MSC_VER)
        #define cut_inlinable __forceinline
    #elif defined(__GNUC__) || defined(__clang__)
        #define cut_inlinable __attribute__((always_inline)) inline
    #endif

    cut_inlinable int cut_is_cpu_big_endian()
    {
        int a = 1;
        return *((char*)&a) == 0;
    }

    cut_inlinable int cut_is_cpu_little_endian()
    {
        int a = 1;
        return *((char*)&a) == 1;
    }

    #define cut_array_size(array) sizeof((array))/sizeof((array)[0])

    #define CUT_WINDOWS 0
    #define CUT_LINUX   1
    #define CUT_MACOS   2

    // You can define CUT_TARGET_OS to one the following constants above or let cutile try to guess it:
    #if !defined(CUT_TARGET_OS)
        #if defined(_WIN32)
            #define CUT_TARGET_OS CUT_WINDOWS
        #elif defined(__MACH__)
            #define CUT_TARGET_OS CUT_MACOS
        #elif defined(__linux__)
            #define CUT_TARGET_OS CUT_LINUX
        #endif
    #endif

    cut_inlinable cut_uptrsize cut_align_nb(cut_uptrsize v, cut_uptrsize a)
    {
        return (v + (a - 1)) & ~(a - 1);
    }

    #if __STDC_VERSION__ >= 201112L
        #define CUT_C11_SUPPORTED 1
    #else
        #define CUT_C11_SUPPORTED 0
    #endif

    #if __STDC_VERSION__ >= 202311L
        #define CUT_C23_SUPPORTED 1
    #else
        #define CUT_C23_SUPPORTED 0
    #endif

    // Compile-time assertion.
    #define cut_ct_assert(expr, msg) _Static_assert(expr, msg)
    #define cut_ct_assert_(expr) _Static_assert(expr, "Unsatisfied condition: "#expr)

    #if defined(CUT_ENABLE_ASSERT) || defined(CUT_DEBUG) || defined(DEBUG)
        #ifdef CUT_ASSERT_HANDLER
            // To define yourself in your own program.
            extern void cut_assert_handler(cut_u8 *msg, cut_u32 msg_len);

            #define CUT_ASSERT__CALL_HANDLER(...) cut_assert_handler(__VA_ARGS__)
        #else
            #define CUT_ASSERT__CALL_HANDLER(...)
        #endif

        #define cut_assert(predicate) {                         \
            if (!(predicate)) {                                 \
                cut_u8 msg[] = #predicate;                      \
                CUT_ASSERT__CALL_HANDLER(msg, sizeof(msg) - 1); \
            }                                                   \
        }
    #else
        #define cut_assert(predicate) (void)0
    #endif

    #define Cut_Arrview(Type) struct {  \
        Type *data;                     \
        cut_uptrsize count;             \
    }

    #define cut_arrview(Array) { .data = Array, .count = cut_array_size(Array) }
    #define cut_arrview_0(Array0) { .data = Array0, .count = cut_array_size(Array0) - 1 }
    #define cut_arrview_ptr(DataPtr, Count) { .data = DataPtr, .count = Count }

    #define cut_arrview_for(ArrViewPtr, ItDecl, Code) {                                                         \
        for (cut_uptrsize cut__arrview_for_i = 0; cut__arrview_for_i < (ArrViewPtr)->count; cut__arrview_for_i++) {  \
            ItDecl = (ArrViewPtr)->data + cut__arrview_for_i;                                                   \
            Code;                                                                                               \
        }                                                                                                       \
    }

    typedef Cut_Arrview(cut_u8) Cut_u8Arrview;

    #define cut_u8arrview(Array) (Cut_u8Arrview){ Array, cut_array_size(Array) }
    #define cut_u8arrview_0(Array0) (Cut_u8Arrview){ Array0, cut_array_size(Array0) - 1 }
    #define cut_u8arrview_ptr(DataPtr, Count) (Cut_u8Arrview){ DataPtr, Count }

    // Makes an array view of a known-size string ending with a 0.
    #define cut_fstr0_ct(FStr0) { .data = (cut_u8 *)FStr0, .count = sizeof(FStr0) - 1 }
    #define cut_fstr0(FStr0)    ((Cut_u8Arrview)cut_fstr0_ct(FStr0))

    #define cut_KB(number) ((number)*1024)
    #define cut_MB(number) (cut_KB(number) * 1024)
    #define cut_GB(number) (cut_MB(number) * 1024)
    #define cut_TB(number) (cut_GB(number) * 1024)

    #define cut_MIN(a, b) ((a) < (b) ? (a) : (b))
    #define cut_MAX(a, b) ((a) > (b) ? (a) : (b))

    #if defined(CUT_BASE_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define internal cut_internal

        #define persist cut_persist

        #define thread_local cut_thread_local

        typedef cut_s8 s8;
        typedef cut_u8 u8;
        typedef cut_s16 s16;
        typedef cut_u16 u16;
        typedef cut_s32 s32;
        typedef cut_u32 u32;
        typedef cut_s64 s64;
        typedef cut_u64 u64;
        typedef cut_f32 f32;
        typedef cut_f64 f64;

        typedef cut_uptrsize uptrsize;
        typedef cut_sptrsize sptrsize;

        #define field_offset    cut_field_offset
        #define field_size      cut_field_size

        #define packed cut_packed

        #define alignas     cut_alignas
        #define cc_alignas  cut_cc_alignas

        #define inlinable cut_inlinable

        #define is_cpu_big_endian       cut_is_cpu_big_endian
        #define is_cpu_little_endian    cut_is_cpu_little_endian

        #define array_size cut_array_size

        #define WINDOWS CUT_WINDOWS
        #define LINUX   CUT_LINUX
        #define MACOS   CUT_MACOS

        #define align_nb cut_align_nb

        #define C11_SUPPORTED CUT_C11_SUPPORTED
        #define C23_SUPPORTED CUT_C23_SUPPORTED

        #define ct_assert   cut_ct_assert
        #define ct_assert_  cut_ct_assert_

        #define assert cut_assert

        #define Arrview Cut_Arrview
        #define arrview cut_arrview
        #define arrview_0 cut_arrview_0
        #define arrview_ptr cut_arrview_ptr

        #define arrview_for cut_arrview_for

        typedef Cut_u8Arrview u8Arrview;

        #define u8arrview     cut_u8arrview
        #define u8arrview_0   cut_u8arrview_0
        #define u8arrview_ptr cut_u8arrview_ptr

        #define fstr0_ct cut_fstr0_ct
        #define fstr0    cut_fstr0

        #define KB cut_KB
        #define MB cut_MB
        #define GB cut_GB
        #define TB cut_TB

        #define MIN cut_MIN
        #define MAX cut_MAX
    #endif
#endif
