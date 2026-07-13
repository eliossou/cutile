#if !defined(CUT_WIN32)
    #define CUT_WIN32

    #if !defined(cut_WIN32_HEADER)
        #define WIN32_LEAN_AND_MEAN
        #define cut_WIN32_HEADER <windows.h>
    #endif
    #include cut_WIN32_HEADER

    #if !defined(cut_WIN32_HANDLE)
        #define cut_WIN32_HANDLE void*
    #endif

    #if !defined(cut_WIN32_BOOL)
        #define cut_WIN32_BOOL int
    #endif

    #if !defined(cut_WIN32_DWORD)
        #define cut_WIN32_DWORD unsigned long
    #endif

    #if !defined(cut_WIN32_DWORD64)
        #define cut_WIN32_DWORD64 DWORD64
    #endif

    #ifndef cut_WIN32_LARGE_INTEGER
        #define cut_WIN32_LARGE_INTEGER LARGE_INTEGER
    #endif

    // Handle API:
    #if !defined(cut_WIN32_STD_OUTPUT_HANDLE)
        #define cut_WIN32_STD_OUTPUT_HANDLE ((cut_WIN32_DWORD)-11)
    #endif

    #if !defined(cut_WIN32_INVALID_HANDLE_VALUE)
        #define cut_WIN32_INVALID_HANDLE_VALUE ((cut_WIN32_HANDLE)-1)
    #endif

    #if !defined(cut_WIN32_GetStdHandle)
        #define cut_WIN32_GetStdHandle GetStdHandle
    #endif

    // File API:

    #if !defined(cut_WIN32_FILE_GENERIC_READ)
        #define cut_WIN32_FILE_GENERIC_READ 0x80000000
    #endif

    #if !defined(cut_WIN32_FILE_GENERIC_WRITE)
        #define cut_WIN32_FILE_GENERIC_WRITE 0x40000000
    #endif

    #if !defined(cut_WIN32_FILE_SHARE_READ)
        #define cut_WIN32_FILE_SHARE_READ 0x00000001
    #endif

    #if !defined(cut_WIN32_FILE_SHARE_WRITE)
        #define cut_WIN32_FILE_SHARE_WRITE 0x00000002
    #endif

    #if !defined(cut_WIN32_CREATE_ALWAYS)
        #define cut_WIN32_CREATE_ALWAYS 2
    #endif

    #if !defined(cut_WIN32_OPEN_ALWAYS)
        #define cut_WIN32_OPEN_ALWAYS 4
    #endif

    #if !defined(cut_WIN32_OPEN_EXISTING)
        #define cut_WIN32_OPEN_EXISTING 3
    #endif

    #if !defined(cut_WIN32_INVALID_HANDLE_VALUE)
        #define cut_WIN32_INVALID_HANDLE_VALUE ((cut_WIN32_HANDLE)-1)
    #endif

    #if !defined(cut_WIN32_CreateFileA)
        #define cut_WIN32_CreateFileA CreateFileA
    #endif

    #if !defined(cut_WIN32_CloseHandle)
        #define cut_WIN32_CloseHandle CloseHandle
    #endif

    #if !defined(cut_WIN32_ReadFile)
        #define cut_WIN32_ReadFile ReadFile
    #endif

    #if !defined(cut_WIN32_SetFilePointerEx)
        #define cut_WIN32_SetFilePointerEx SetFilePointerEx
    #endif

    #if !defined(cut_WIN32_FILE_BEGIN)
        #define cut_WIN32_FILE_BEGIN 0
    #endif

    #if !defined(cut_WIN32_FILE_CURRENT)
        #define cut_WIN32_FILE_CURRENT 1
    #endif

    #if !defined(cut_WIN32_FILE_END)
        #define cut_WIN32_FILE_END 2
    #endif

    #if !defined(cut_WIN32_GetFileSizeEx)
        #define cut_WIN32_GetFileSizeEx GetFileSizeEx
    #endif

    #if !defined(cut_WIN32_ReadFile)
        #define cut_WIN32_ReadFile ReadFile
    #endif

    #if !defined(cut_WIN32_WriteFile)
        #define cut_WIN32_WriteFile WriteFile
    #endif

    // Debug API:

    #if !defined(cut_WIN32_DBGHELP_HEADER)
        #define cut_WIN32_DBGHELP_HEADER <dbghelp.h>
    #endif

    #if !defined(cut_WIN32_CONTEXT)
        #define cut_WIN32_CONTEXT CONTEXT
    #endif

    #if !defined(cut_WIN32_STACKFRAME64)
        #define cut_WIN32_STACKFRAME64 STACKFRAME64
    #endif

    #if !defined(cut_win32_SYMBOL_INFO)
        #define cut_win32_SYMBOL_INFO SYMBOL_INFO
    #endif

    #if !defined(cut_win32_IMAGEHLP_LINE64)
        #define cut_win32_IMAGEHLP_LINE64 IMAGEHLP_LINE64
    #endif

    #if !defined(cut_win32_SymInitialize)
        #define cut_win32_SymInitialize SymInitialize
    #endif

    #if !defined(cut_win32_GetCurrentProcess)
        #define cut_win32_GetCurrentProcess GetCurrentProcess
    #endif

    #if !defined(cut_win32_GetCurrentThread)
        #define cut_win32_GetCurrentThread GetCurrentThread
    #endif

    #if !defined(cut_win32_RtlCaptureContext)
        #define cut_win32_RtlCaptureContext RtlCaptureContext
    #endif

    #if !defined(cut_win32_StackWalk)
        #define cut_win32_StackWalk StackWalk
    #endif

    #if !defined(cut_win32_IMAGE_FILE_MACHINE_AMD64)
        #define cut_win32_IMAGE_FILE_MACHINE_AMD64 IMAGE_FILE_MACHINE_AMD64
    #endif

    #if !defined(cut_win32_SymFromAddr)
        #define cut_win32_SymFromAddr SymFromAddr
    #endif

    #if !defined(cut_win32_SymGetLineFromAddr)
        #define cut_win32_SymGetLineFromAddr SymGetLineFromAddr
    #endif

    #if !defined(cut_win32_SymFunctionTableAccess64)
        #define cut_win32_SymFunctionTableAccess64 SymFunctionTableAccess64
    #endif

    #if !defined(cut_win32_SymGetModuleBase64)
        #define cut_win32_SymGetModuleBase64 SymGetModuleBase64
    #endif

    // Memory API:

    #if !defined(cut_WIN32_VirtualAlloc)
        #define cut_WIN32_VirtualAlloc VirtualAlloc
    #endif

    #if !defined(cut_WIN32_VirtualFree)
        #define cut_WIN32_VirtualFree VirtualFree
    #endif

    #if !defined(CUT_WIN32_MEM_COMMIT)
        #define CUT_WIN32_MEM_COMMIT 0x00001000
    #endif

    #if !defined(CUT_WIN32_MEM_RESERVE)
        #define CUT_WIN32_MEM_RESERVE 0x00002000
    #endif

    #if !defined(CUT_WIN32_PAGE_NOACCESS)
        #define CUT_WIN32_PAGE_NOACCESS 0x01
    #endif

    #if !defined(CUT_WIN32_PAGE_READWRITE)
        #define CUT_WIN32_PAGE_READWRITE 0x04
    #endif

    #if !defined(CUT_WIN32_MEM_DECOMMIT)
        #define CUT_WIN32_MEM_DECOMMIT 0x00004000
    #endif

    #if !defined(CUT_WIN32_MEM_RELEASE)
        #define CUT_WIN32_MEM_RELEASE 0x00008000
    #endif
#endif
