#include "../ini.c"

// ct variant is called to have a constant expression.
u8Arrview ini_data = fstr0_ct(
    "Global=50\n"
    "[SECTION]\n"
    "ABC=23\n"
    "B=FOO\n"
);

void run_ini_tests()
{
    Parse_Ini_Result ini = ini_parse(ini_data, memallocator);

    test(ini.error.msg.data == 0);
    if (ini.error.msg.data) {
        print(ini.error.msg);
    }

    Ini_Get_Entry_Result entry;

    entry = ini_get_global_entry_value(&ini, fstr0("Global"));
    test(entry.found);
    test(ini_data.data[entry.value_start] == '5');
    test(ini_data.data[entry.value_start + entry.value_size - 1] == '0');

    entry = ini_get_entry_value(&ini, fstr0("SECTION"), fstr0("ABC"));
    test(entry.found);
    test(ini_data.data[entry.value_start] == '2');
    test(ini_data.data[entry.value_start + entry.value_size - 1] == '3');

    entry = ini_get_entry_value(&ini, fstr0("SECTION"), fstr0("B"));
    test(entry.found);
    test(ini_data.data[entry.value_start] == 'F');
    test(ini_data.data[entry.value_start + entry.value_size - 1] == 'O');

    ini_destroy_parsed_data(&ini);

    Ini_Field field;
    test(ini_get_global_field(ini_data, fstr0("Global"), &field));
}
