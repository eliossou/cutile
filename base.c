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
    #endif

    #ifdef _MSC_VER
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
    #else
        cut_ct_assert(0, "Unknown platform pointer size.")
    #endif

    #define cut_field_offset(StructureType, FieldName) (cut_uptrsize)(&(((StructureType *)0)->FieldName))
    #define cut_field_size(StructType, FieldName) sizeof(((StructType *)0)->FieldName)

    #if defined(__GNUC__)
        #define cut_packed(Declaration) Declaration __attribute__((__packed__))
    #elif defined(_MSC_VER)
        #define cut_packed(Declaration) __pragma(pack(push, 1)) Declaration __pragma(pack(pop))
    #endif

    #define cut_alignas(AlignmentOrType) _Alignas(AlignmentOrType)

    #define cut_inlinable static inline

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

    cut_inlinable cut_u64 cut_align_nb(cut_u64 v, cut_u64 a)
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

    #if defined(CUT_ENABLE_ASSERT)
        // To define yourself in your own program.
        extern void cut_assert_proc(cut_u8 *msg, cut_u32 msg_len);

        #define cut_assert(predicate) {                 \
            if (!(predicate)) {                         \
                cut_u8 msg[] = #predicate;              \
                cut_assert_proc(msg, sizeof(msg) - 1);  \
            }                                           \
        }
    #else
        #define cut_assert(predicate) (void)0
    #endif

    #define Cut_Arrview(Type) struct {  \
        Type *data;                     \
        cut_u64 count;                  \
    }

    #define cut_arrview(Array) { .data = Array, .count = cut_array_size(Array) }
    #define cut_arrview_0(Array0) { .data = Array0, .count = cut_array_size(Array0) - 1 }
    #define cut_arrview_ptr(DataPtr, Count) { .data = DataPtr, .count = Count }

    #define cut_arrview_for(ArrViewPtr, ItDecl, Code) {                                                         \
        for (cut_u64 cut__arrview_for_i = 0; cut__arrview_for_i < (ArrViewPtr)->count; cut__arrview_for_i++) {  \
            ItDecl = (ArrViewPtr)->data + cut__arrview_for_i;                                                   \
            Code;                                                                                               \
        }                                                                                                       \
    }

    typedef Cut_Arrview(cut_u8) cut_u8arrview;  // @TODO: Replace by Cut_u8Arrview
    typedef cut_u8arrview cut_u8Arrview;        // @REMOVE
    typedef cut_u8arrview Cut_u8Arrview;

    #define cut_u8arrview(Array) (Cut_u8Arrview){ Array, cut_array_size(Array) }
    #define cut_u8arrview_0(Array0) (Cut_u8Arrview){ Array0, cut_array_size(Array0) - 1 }
    #define cut_u8arrview_ptr(DataPtr, Count) (cut_u8arrview){ DataPtr, Count }

    // Makes a Cut_u8Arrview from a pointer to an 0 terminated u8 array.
    cut_inlinable Cut_u8Arrview cut_u8arrview_0p(void *p)
    {
        Cut_u8Arrview result = {
            .data = p
        };

        cut_u8 *p2 = p;
        while (p2[result.count]) {
            result.count++;
        }

        return result;
    }

    cut_inlinable int cut_u8arrview_eq(Cut_u8Arrview *l, Cut_u8Arrview *r)
    {
        if (l->count != r->count)
            return 0;

        // @SPEED
        for (cut_u32 i = 0; i < l->count; i++) {
            if (l->data[i] != r->data[i])
                return 0;
        }

        return 1;
    }

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

        #define alignas cut_alignas

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

        #define ct_assert cut_ct_assert

        #define assert cut_assert

        #define Arrview Cut_Arrview
        #define arrview cut_arrview
        #define arrview_0 cut_arrview_0
        #define arrview_ptr cut_arrview_ptr

        #define arrview_for cut_arrview_for

        typedef cut_u8arrview u8arrview; // @TODO: Remove
        typedef cut_u8Arrview u8Arrview;

        #define u8arrview     cut_u8arrview
        #define u8arrview_0   cut_u8arrview_0
        #define u8arrview_ptr cut_u8arrview_ptr
        #define u8arrview_0p  cut_u8arrview_0p

        #define u8arrview_eq cut_u8arrview_eq

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
