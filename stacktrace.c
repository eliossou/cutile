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
            if (!cut_WIN32_SymInitialize(cut_WIN32_GetCurrentProcess(), 0, 1))
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
            cut_WIN32_HANDLE current_process = cut_WIN32_GetCurrentProcess();
            cut_WIN32_HANDLE current_thread = cut_WIN32_GetCurrentThread();
            cut_WIN32_CONTEXT context;
            cut_WIN32_STACKFRAME64 stackframe = {0};

            cut_WIN32_RtlCaptureContext(&context);

            #if defined(_M_AMD64)
                stackframe.AddrPC.Offset = context.Rip;
                stackframe.AddrFrame.Offset = context.Rbp;
                stackframe.AddrStack.Offset = context.Rsp;
            #elif defined(_M_IX86)
                stackframe.AddrPC.Offset = context.Eip;
                stackframe.AddrFrame.Offset = context.Ebp;
                stackframe.AddrStack.Offset = context.Esp;
            #elif defined(_M_ARM64)
                stackframe.AddrPC.Offset = context.Pc;
                stackframe.AddrFrame.Offset = context.Fp;
                stackframe.AddrStack.Offset = context.Sp;
            #endif
            stackframe.AddrPC.Mode = AddrModeFlat;
            stackframe.AddrFrame.Mode = AddrModeFlat;
            stackframe.AddrStack.Mode = AddrModeFlat;

            #if defined(_M_AMD64)
                #define cut_STACKTRACE_MACHINE_TYPE IMAGE_FILE_MACHINE_AMD64
            #elif defined(_M_IX86)
                #define cut_STACKTRACE_MACHINE_TYPE IMAGE_FILE_MACHINE_I386
            #elif defined(_M_ARM64)
                #define cut_STACKTRACE_MACHINE_TYPE IMAGE_FILE_MACHINE_ARM64
            #endif

            while (cut_WIN32_StackWalk(
                cut_STACKTRACE_MACHINE_TYPE,
                current_process,
                current_thread,
                &stackframe,
                &context,
                0,
                &cut_WIN32_SymFunctionTableAccess64,
                &cut_WIN32_SymGetModuleBase64,
                0
            )) {
                if (skip) {
                    skip--;
                    continue;
                }

                if (max_frames_count == *frames_count)
                    return 0;

                Cut_Stacktrace_Frame *frame = frames + *frames_count;

                #if defined(_M_AMD64)
                    frame->program_counter = (void*)context.Rip;
                #elif defined(_M_IX86)
                    frame->program_counter = (void*)context.Eip;
                #elif defined(_M_ARM64)
                    frame->program_counter = (void*)context.Pc;
                #endif

                cut_WIN32_DWORD64           displacement;
                cut_WIN32_DWORD             line_displacement;
                cut_u8                      symbol_info_buf[sizeof(cut_WIN32_SYMBOL_INFO) + sizeof(frame->routine_name)];
                cut_WIN32_SYMBOL_INFO*      psymbol_info = (cut_WIN32_SYMBOL_INFO*)symbol_info_buf;
                cut_WIN32_IMAGEHLP_LINE64   line_info;

                psymbol_info->SizeOfStruct = sizeof(cut_WIN32_SYMBOL_INFO);
                psymbol_info->MaxNameLen = sizeof(frame->routine_name);

                line_info.SizeOfStruct = sizeof(cut_WIN32_IMAGEHLP_LINE64);

                if (cut_WIN32_SymFromAddr(
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

                if (cut_WIN32_SymGetLineFromAddr(
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
