#include "../file.c"

void run_file_tests()
{
    File f;

    test(open_file_0(&f, fstr0("file_tests.c"), 0, 1));
    close_file(f);

    test(open_file(&f, fstr0("file_tests.c"), 0, 1));

    test(set_file_position(f, 10));

    cut_u64 pos;
    test(get_file_position(f, &pos));
    test(pos == 10);

    close_file(f);

    u8arrview read_content;
    u8arrview expected_content = fstr0("AAA.");

    test(cut_read_entire_file_named(fstr0("file_tests_file"), &read_content, &memallocator));
    test(read_content.count == expected_content.count);
    test(!mem_cmp(read_content.data, expected_content.data, read_content.count));

    Cut_File file;
    test(cut_open_file_0(&file, fstr0("file_tests_file"), 0, 1));

    u64 size;
    test(cut_get_file_size(file, &size));
    test(size == 4);

    cut_close_file(file);
}
