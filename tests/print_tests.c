#include "../print.c"

void run_print_tests()
{
    cut_u8Arrview r;

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(0));
    test(r.count == 1 && r.data[0] == '0');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(-5));
    test(r.count == 2 && r.data[0] == '-' && r.data[1] == '5');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(12345));
    test(r.count == 5);
    test(r.data[0] == '1' && r.data[4] == '5');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int(-12345));
    test(r.count == 6);
    test(r.data[0] == '-' && r.data[1] == '1' && r.data[5] == '5');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int_hex(0));
    test(r.count == 3);
    test(r.data[0] == '0' && r.data[1] == 'x' && r.data[2] == '0');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_int_hex(255));
    test(r.count == 4);
    test(r.data[2] == 'F' && r.data[3] == 'F');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_address(&r));
    test(r.count > 2 || r.count <= 18);
    test(r.data[0] == '0' && r.data[1] == 'x');
    cut_mem_free(r.data, &memallocator);

    r = cut_sprint(&memallocator, cut_fstr0("%=%"), cut_format_str(fstr0("key")), cut_format_int(42));
    test(r.count == 6);
    test(r.data[0] == 'k' && r.data[2] == 'y' && r.data[3] == '=' && r.data[4] == '4' && r.data[5] == '2');
    cut_mem_free(r.data, &memallocator);

    // Test format_str0 (null-terminated string).
    r = cut_sprint(&memallocator, cut_fstr0("%"), cut_format_str0("hi"));
    test(r.count == 2 && r.data[0] == 'h' && r.data[1] == 'i');
    cut_mem_free(r.data, &memallocator);
}
