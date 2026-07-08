#include "../stacktrace.c"

void run_stacktrace_tests()
{
    Cut_Stacktrace_Frame frames[10];
    int frames_nb;
    test(stacktrace(frames, &frames_nb, array_size(frames), 0));

    test(frames_nb >= 2);
}
