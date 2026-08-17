#ifndef CUT_PP
    #define CUT_PP

    // Counts the number of arguments passed to (...) at compile time.
    // Returns an integer constant expression in range [1, 16].
    // Requires at least one argument.
    #define cut_va_args_count(...) cut_va_args_count__(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

    #define cut_va_args_count__( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                                 N, ...) N

    #if defined(CUT_PP_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define va_args_count cut_va_args_count
    #endif
#endif