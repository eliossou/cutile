// Example: Capturing and printing a stacktrace using cutile's stacktrace API.
//
// Compile on Windows with MSVC: cl example_stacktrace.c /link Dbghelp.lib
// Compile on Windows with MinGW: gcc example_stacktrace.c -ldbghelp -o example_stacktrace.exe

#define CUT_IMPL
#define CUT_SHORT_NAMES
#include "../base.c"
#include "../print.c"
#include "../stacktrace.c"

void print_stacktrace()
{
    Stacktrace_Frame frames[16];
    int frames_count;

    stacktrace(frames, &frames_count, 16, 1);

    for (int i = 0; i < frames_count; i++) {
        Stacktrace_Frame *fr = frames + i;
        u8Arrview routine_name = u8arrview_ptr(fr->routine_name, fr->routine_name_length);
        u8Arrview filename = u8arrview_ptr(fr->filename, fr->filename_length);

        print(
            fstr0("  #% at % in %:%:%\n"),
            format_int(i),
            format_address(fr->program_counter),
            format_str(routine_name),
            format_str(filename),
            format_int(fr->line)
        );
    }
}

void func2()
{
    print_stacktrace();
}

void func1()
{
    func2();
}

int main()
{
    if (!stacktrace_api_init()) {
        print(fstr0("Failed to initialize stacktrace API.\n"));
        return 1;
    }

    func1();

    return 0;
}
