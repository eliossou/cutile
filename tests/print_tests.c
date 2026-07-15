#include "../print.c"

void run_print_tests()
{
    cut_u8Arrview r;

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(0));
    test(u8arrview_eq(&fstr0("0"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(-5));
    test(u8arrview_eq(&fstr0("-5"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(12345));
    test(u8arrview_eq(&fstr0("12345"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(-12345));
    test(u8arrview_eq(&fstr0("-12345"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int_hex(0));
    test(u8arrview_eq(&fstr0("0x0000000000000000"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int_hex(255));
    test(u8arrview_eq(&fstr0("0x00000000000000FF"), &r));
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_address(&r));
    test(r.count == 18);
    test(r.data[0] == '0' && r.data[1] == 'x');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%=%"), cut_format_str(fstr0("key")), cut_format_int(42));
    test(u8arrview_eq(&fstr0("key=42"), &r));
    cut_mem_free(r.data, &memallocator);

    // Test format_str0 (null-terminated string).
    r = cut_sprint(&memallocator, fstr0("%"), format_str0("hi"));
    test(u8arrview_eq(&fstr0("hi"), &r));
    mem_free(r.data, &memallocator);

    // Negative number boundaries
    r = sprint(&memallocator, fstr0("%"), format_int(-1));
    test(u8arrview_eq(&fstr0("-1"), &r));
    mem_free(r.data, &memallocator);

    r = sprint(&memallocator, fstr0("%"), format_int(-2147483647 - 1));
    test(u8arrview_eq(&fstr0("-2147483648"), &r));
    mem_free(r.data, &memallocator);

    r = sprint(&memallocator, fstr0("%"), format_int(9223372036854775807));
    test(u8arrview_eq(&fstr0("9223372036854775807"), &r));
    mem_free(r.data, &memallocator);

    // Negative hex
    r = sprint(&memallocator, fstr0("%"), format_int_hex(-255));
    test(u8arrview_eq(&fstr0("0xFFFFFFFFFFFFFF01"), &r));
    mem_free(r.data, &memallocator);

    r = sprint(&memallocator, fstr0("%"), format_int_hex(-1));
    test(u8arrview_eq(&fstr0("0xFFFFFFFFFFFFFFFF"), &r));
    mem_free(r.data, &memallocator);
}
