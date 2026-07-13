#ifndef CUT_STACKTRACE
    #define CUT_STACKTRACE

    #include "base.c"

    typedef struct Cut_Stacktrace_Frame {
        int line;
        cut_u8 filename[128];
        cut_u8 routine_name[64];
        cut_u32 filename_length;
        cut_u32 routine_name_length;

        void *program_counter;
    } Cut_Stacktrace_Frame;

    int cut_stacktrace_api_init();

    // On Linux and macOS, a max_frames_count+skip up to 32 is supported.
    int cut_stacktrace(Cut_Stacktrace_Frame *frames, int *frames_count, int max_frames_count, int skip);

    #ifdef CUT_SHORT_NAMES
        #define stacktrace_api_init() cut_stacktrace_api_init()
        #define stacktrace(frames, frames_count, max_frames_count, skip) cut_stacktrace(frames, frames_count, max_frames_count, skip)

        typedef Cut_Stacktrace_Frame Stacktrace_Frame;
    #endif
#endif

#if (defined(CUT_STACKTRACE_IMPL) || defined(CUT_IMPL)) && !defined(CUT_STACKTRACE_IMPL_INCLUDED)
    #define CUT_STACKTRACE_IMPL_INCLUDED

    #include "memory.c"

    #if CUT_TARGET_OS == CUT_WINDOWS
        #include "win32.c"
        #include cut_WIN32_DBGHELP_HEADER
        #ifdef _MSC_VER
            #pragma comment(lib, "DbgHelp.lib")
        #endif
    #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        #include <execinfo.h>
        #include <dlfcn.h>
    #endif

    int cut_stacktrace_api_init()
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
            if (!cut_win32_SymInitialize(cut_win32_GetCurrentProcess(), 0, 1))
                return 0;
        #endif

        return 1;
    }

    int cut_stacktrace(Cut_Stacktrace_Frame *frames, int *frames_count, int max_frames_count, int skip)
    {
        int current_skip = 0;
        *frames_count = 0;

        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_WIN32_HANDLE current_process = cut_win32_GetCurrentProcess();
            cut_WIN32_HANDLE current_thread = cut_win32_GetCurrentThread();
            cut_WIN32_CONTEXT context;
            cut_WIN32_STACKFRAME64 stackframe = {0};

            cut_win32_RtlCaptureContext(&context);

            stackframe.AddrPC.Offset = context.Rip;
            stackframe.AddrPC.Mode = AddrModeFlat;
            stackframe.AddrFrame.Offset = context.Rbp;
            stackframe.AddrFrame.Mode = AddrModeFlat;
            stackframe.AddrStack.Offset = context.Rsp;
            stackframe.AddrStack.Mode = AddrModeFlat;

            while (cut_win32_StackWalk(
                cut_win32_IMAGE_FILE_MACHINE_AMD64,
                current_process,
                current_thread,
                &stackframe,
                &context,
                0,
                &cut_win32_SymFunctionTableAccess64,
                &cut_win32_SymGetModuleBase64,
                0
            )) {
                if (skip) {
                    skip--;
                    continue;
                }

                if (max_frames_count == *frames_count)
                    return 0;

                Cut_Stacktrace_Frame *frame = frames + *frames_count;

                frame->program_counter = (void*)context.Rip;

                cut_WIN32_DWORD64           displacement;
                cut_WIN32_DWORD             line_displacement;
                cut_u8                      symbol_info_buf[sizeof(cut_win32_SYMBOL_INFO) + sizeof(frame->routine_name)];
                cut_win32_SYMBOL_INFO*      psymbol_info = (cut_win32_SYMBOL_INFO*)symbol_info_buf;
                cut_win32_IMAGEHLP_LINE64   line_info;

                psymbol_info->SizeOfStruct = sizeof(cut_win32_SYMBOL_INFO);
                psymbol_info->MaxNameLen = sizeof(frame->routine_name);

                line_info.SizeOfStruct = sizeof(cut_win32_IMAGEHLP_LINE64);

                if (cut_win32_SymFromAddr(
                    current_process,
                    stackframe.AddrPC.Offset,
                    &displacement,
                    psymbol_info
                )) {
                    if (psymbol_info->NameLen > sizeof(frame->routine_name))
                        frame->routine_name_length = sizeof(frame->routine_name);
                    else
                        frame->routine_name_length = psymbol_info->NameLen;
                    cut_mem_cpy(frame->routine_name, psymbol_info->Name, frame->routine_name_length);
                }
                else {
                    frame->routine_name_length = 0;
                }

                if (cut_win32_SymGetLineFromAddr(
                    current_process,
                    stackframe.AddrPC.Offset,
                    &line_displacement,
                    &line_info
                )) {
                    frame->line = line_info.LineNumber;

                    frame->filename_length = 0;
                    while (line_info.FileName[frame->filename_length]) {
                        frame->filename[frame->filename_length] = line_info.FileName[frame->filename_length];
                        frame->filename_length++;

                        if (frame->filename_length == sizeof(frame->filename))
                            break;
                    }
                }
                else {
                    frame->line = -1;
                    frame->filename_length = 0;
                }

                (*frames_count)++;
            }
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            // @TODO: Instruction line is not resolved. Same for the file name.

            cut_persist void *addresses[32];
            int address_count = backtrace(addresses, max_frames_count);

            Dl_info info;
            int j;

            for (int i = 0; i < address_count - skip; i++) {
                frames[i].program_counter = addresses[i+skip];
                frames[i].filename_length = 0;
                frames[i].line = 0;

                dladdr(frames[i].program_counter, &info);
                if (info.dli_sname) {
                    for (j = 0; info.dli_sname[j]; j++) {
                        frames[i].routine_name[j] = info.dli_sname[j];
                    }
                }
                frames[i].routine_name_length = j;

                (*frames_count)++;
            }
        }
        #endif

        return 1;
    }
#endif
