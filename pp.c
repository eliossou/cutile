#ifndef CUT_PP
    #define CUT_PP

    // Counts the number of arguments passed to (...) at compile time.
    // Returns an integer constant expression in range [1, 16].
    // Requires at least one argument.
    #define cut_va_args_count(...) cut_va_args_count__(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

    #define cut_va_args_count__( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                                 N, ...) N

    // Applies a macro to each argument passed to (...), expanding to:
    //     apply(arg1) apply(arg2) ...
    // Requires at least one argument after the apply macro. Supports up to 16 arguments.
    #define cut_va_args_for_each(apply, ...) \
        cut_va_args_for_each__(apply, __VA_ARGS__, \
            cut_va_args_for_each_16, cut_va_args_for_each_15, cut_va_args_for_each_14, cut_va_args_for_each_13, \
            cut_va_args_for_each_12, cut_va_args_for_each_11, cut_va_args_for_each_10, cut_va_args_for_each_9, \
            cut_va_args_for_each_8, cut_va_args_for_each_7, cut_va_args_for_each_6, cut_va_args_for_each_5, \
            cut_va_args_for_each_4, cut_va_args_for_each_3, cut_va_args_for_each_2, cut_va_args_for_each_1) \
        (apply, __VA_ARGS__)

    #define cut_va_args_for_each__( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, \
                                    apply, ...) apply

    #define cut_va_args_for_each_1(apply, x, ...) apply(x)
    #define cut_va_args_for_each_2(apply, x, ...) apply(x) cut_va_args_for_each_1(apply, __VA_ARGS__)
    #define cut_va_args_for_each_3(apply, x, ...) apply(x) cut_va_args_for_each_2(apply, __VA_ARGS__)
    #define cut_va_args_for_each_4(apply, x, ...) apply(x) cut_va_args_for_each_3(apply, __VA_ARGS__)
    #define cut_va_args_for_each_5(apply, x, ...) apply(x) cut_va_args_for_each_4(apply, __VA_ARGS__)
    #define cut_va_args_for_each_6(apply, x, ...) apply(x) cut_va_args_for_each_5(apply, __VA_ARGS__)
    #define cut_va_args_for_each_7(apply, x, ...) apply(x) cut_va_args_for_each_6(apply, __VA_ARGS__)
    #define cut_va_args_for_each_8(apply, x, ...) apply(x) cut_va_args_for_each_7(apply, __VA_ARGS__)
    #define cut_va_args_for_each_9(apply, x, ...) apply(x) cut_va_args_for_each_8(apply, __VA_ARGS__)
    #define cut_va_args_for_each_10(apply, x, ...) apply(x) cut_va_args_for_each_9(apply, __VA_ARGS__)
    #define cut_va_args_for_each_11(apply, x, ...) apply(x) cut_va_args_for_each_10(apply, __VA_ARGS__)
    #define cut_va_args_for_each_12(apply, x, ...) apply(x) cut_va_args_for_each_11(apply, __VA_ARGS__)
    #define cut_va_args_for_each_13(apply, x, ...) apply(x) cut_va_args_for_each_12(apply, __VA_ARGS__)
    #define cut_va_args_for_each_14(apply, x, ...) apply(x) cut_va_args_for_each_13(apply, __VA_ARGS__)
    #define cut_va_args_for_each_15(apply, x, ...) apply(x) cut_va_args_for_each_14(apply, __VA_ARGS__)
    #define cut_va_args_for_each_16(apply, x, ...) apply(x) cut_va_args_for_each_15(apply, __VA_ARGS__)

    #if defined(CUT_PP_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define va_args_count cut_va_args_count
        #define va_args_for_each cut_va_args_for_each
    #endif
#endif