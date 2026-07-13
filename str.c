#ifndef CUT_STR
    #define CUT_STR

    #include "base.c"

    cut_inlinable int cut_str0_len(void *s)
    {
        int i = 0;
        while (((cut_u8 *)s)[i])
            i++;
        return i;
    }

    cut_inlinable int cut_str0_eq(void *l, void *r)
    {
        cut_u8 *l2 = l;
        cut_u8 *r2 = r;
        while (1) {
            if (*l2 != *r2)
                return 0;
            if (*l2 == 0)
                return 1;
            l2++;
            r2++;
        }
    }

    cut_inlinable cut_u8Arrview cut_str0v(void *s)
    {
        cut_u8Arrview view = {
            .data = s,
            .count = cut_str0_len(s)
        };

        return view;
    }

    #if defined(CUT_STR_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define str0_len cut_str0_len
        #define str0_eq  cut_str0_eq
        #define str0v    cut_str0v
    #endif
#endif
