#ifndef CUT_FILE
    #define CUT_FILE

    #include "base.c"
    #include "memory.c"

    #if CUT_TARGET_OS == CUT_WINDOWS
        #include "win32.c"

        typedef struct Cut_File {
            cut_WIN32_HANDLE handle;
        } Cut_File;
    #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        typedef struct Cut_File {
            int handle;
        } Cut_File;
    #endif

    // Defines the buffer size use to construct 0-ended strings.
    #if !defined(CUT_FILE_PATH0_BUF_SIZE)
        #define CUT_FILE_PATH0_BUF_SIZE 255
    #endif

    // "name" size must not be superior than CUT_FILE_PATH0_BUF_SIZE.
    int cut_open_file(Cut_File *out, cut_u8Arrview name, int writable, int keep_content);
    int cut_open_file_0(Cut_File *out, cut_u8Arrview name0, int writable, int keep_content);

    void cut_close_file(Cut_File file);

    int cut_set_file_position(Cut_File file, cut_u64 position);
    int cut_get_file_position(Cut_File file, cut_u64 *pos);

    int cut_get_file_size(Cut_File file, cut_u64 *size);
    // "name" size must not be superior than CUT_FILE_PATH0_BUF_SIZE.
    int cut_get_file_size_named(cut_u8Arrview name, cut_u64 *size);
    int cut_get_file_size_named_0(cut_u8Arrview name0, cut_u64 *size);

    int cut_read_file(Cut_File file, cut_u64 size, cut_u8Arrview *out, Cut_Mem_Allocator *mem_allocator);
    // "out" is a pre-allocated buffer. It needs to have enough memory to store the whole file content.
    int cut_read_file2(Cut_File file, cut_u64 size, cut_u8Arrview *out);

    int cut_read_entire_file(Cut_File file, cut_u8Arrview *out, Cut_Mem_Allocator *mem_allocator);
    // "name" size must not be superior than CUT_FILE_PATH0_BUF_SIZE.
    int cut_read_entire_file_named(cut_u8arrview name, cut_u8Arrview *out, Cut_Mem_Allocator *mem_allocator);
    int cut_read_entire_file_named_0(cut_u8arrview name0, cut_u8Arrview *out, Cut_Mem_Allocator *mem_allocator);

    int cut_write_file(Cut_File file, Cut_u8Arrview data);
    // "name" size must not be superior than CUT_FILE_PATH0_BUF_SIZE.
    int cut_write_file_named(Cut_u8Arrview name, Cut_u8Arrview data);
    int cut_write_file_named_0(Cut_u8Arrview name0, Cut_u8Arrview data);

    #if defined(CUT_FILE_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        typedef Cut_File File;

        #define open_file   cut_open_file
        #define open_file_0 cut_open_file_0

        #define close_file cut_close_file

        #define set_file_position cut_set_file_position
        #define get_file_position cut_get_file_position

        #define get_file_size           cut_get_file_size
        #define get_file_size_named     cut_get_file_size_named
        #define get_file_size_named_0   cut_get_file_size_named_0

        #define read_file   cut_read_file
        #define read_file2  cut_read_file2

        #define read_entire_file            cut_read_entire_file
        #define read_entire_file_named      cut_read_entire_file_named
        #define read_entire_file_named_0    cut_read_entire_file_named_0

        #define write_file         cut_write_file
        #define write_file_named   cut_write_file_named
        #define write_file_named_0 cut_write_file_named_0
    #endif
#endif

#if (defined(CUT_IMPL) || defined(CUT_FILE_IMPL)) && !defined(CUT_FILE_IMPL_INCLUDED)
    #define CUT_FILE_IMPL_INCLUDED

    #if CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        #include <unistd.h>
        #include <fcntl.h>
    #endif

    int cut_open_file(Cut_File *out, cut_u8arrview name, int writable, int keep_content)
    {
        #if CUT_TARGET_OS == CUT_MACOS || CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_WINDOWS
        {
            // Thoses operating systems APIs require 0 ended strings.
            // So we make a 0 ending string with the help of another buffer.
            // @NOTE: Windows has NtCreateFile in Win32, where you can specify the string size but it uses 16-bit unicode
            //        and the efforts required to make a 16-bit unicode character string are not worth it.

            cut_persist cut_u8 path0_buf[CUT_FILE_PATH0_BUF_SIZE+1];

            cut_u64 size = name.count + 1;
            if (size > cut_array_size(path0_buf))
                return 0;   // Buffer is too short to welcome our path. You can change CUT_FILE_PATH0_BUF_SIZE to a higher value.

            cut_mem_cpy(path0_buf, name.data, name.count);
            path0_buf[name.count] = '\0';

            return cut_open_file_0(out, cut_u8arrview_ptr(path0_buf, size), writable, keep_content);
        }
        #endif
    }

    int cut_open_file_0(Cut_File *out, cut_u8arrview name0, int writable, int keep_content)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            if (writable) {
                if (keep_content) {
                    out->handle = cut_WIN32_CreateFileA(
                        name0.data, cut_WIN32_FILE_GENERIC_READ | cut_WIN32_FILE_GENERIC_WRITE, cut_WIN32_FILE_SHARE_READ,
                        0, cut_WIN32_OPEN_ALWAYS, 0, 0
                    );
                } else {
                    out->handle = cut_WIN32_CreateFileA(
                        name0.data, cut_WIN32_FILE_GENERIC_READ | cut_WIN32_FILE_GENERIC_WRITE, cut_WIN32_FILE_SHARE_READ,
                        0, cut_WIN32_CREATE_ALWAYS, 0, 0
                    );
                }
            } else {
                out->handle = cut_WIN32_CreateFileA(name0.data, cut_WIN32_FILE_GENERIC_READ, cut_WIN32_FILE_SHARE_READ, 0, cut_WIN32_OPEN_EXISTING, 0, 0);
            }

            if (out->handle == cut_WIN32_INVALID_HANDLE_VALUE)
                return 0;

            return 1;
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            int flags;
            if (writable)
                flags = O_RDWR;
            else
                flags = O_RDONLY;
            if (!keep_content)
                flags |= O_TRUNC;

            out->handle = open((char *)name0.data, flags);

            if (out->handle == -1)
                return 0;

            return 1;
        }
        #endif
    }

    void cut_close_file(Cut_File file)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_WIN32_CloseHandle(file.handle);
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            close(file.handle);
        }
        #endif
    }

    int cut_set_file_position(Cut_File file, cut_u64 position)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_WIN32_LARGE_INTEGER li;
            li.QuadPart = position;
            return cut_WIN32_SetFilePointerEx(file.handle, li, 0, cut_WIN32_FILE_BEGIN);
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            if (lseek(file.handle, position, SEEK_SET) == -1)
                return 0;
            return 1;
        }
        #endif
    }

    int cut_get_file_position(Cut_File file, cut_u64 *pos)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_WIN32_LARGE_INTEGER li;
            if (!cut_WIN32_SetFilePointerEx(file.handle, (cut_WIN32_LARGE_INTEGER){ .QuadPart = 0 }, &li, cut_WIN32_FILE_CURRENT))
                return 0;
            *pos = (cut_u64)li.QuadPart;
            return 1;
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            off_t curr_pos = lseek(file.handle, 0, SEEK_CUR);
            if (curr_pos == -1)
                return 0;
            *pos = (cut_u64)curr_pos;
            return 1;
        }
        #endif
    }

    int cut_get_file_size(Cut_File file, cut_u64 *size)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_WIN32_LARGE_INTEGER li;
            if (!cut_WIN32_GetFileSizeEx(file.handle, &li))
                return 0;
            *size = (cut_u64)li.QuadPart;
            return 1;
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            off_t curr_pos = lseek(file.handle, 0, SEEK_CUR);
            if (curr_pos == -1)
                return 0;

            off_t end_pos = lseek(file.handle, 0, SEEK_END);
            if (end_pos == -1)
                return 0;

            lseek(file.handle, curr_pos, SEEK_SET);

            *size = (cut_u64)end_pos;

            return 1;
        }
        #endif
    }

    int cut_get_file_size_named(cut_u8arrview name, cut_u64 *size)
    {
        Cut_File file;

        if (!cut_open_file(&file, name, 0, 1))
            return 0;

        int get_size_success = cut_get_file_size(file, size);

        cut_close_file(file);

        return get_size_success;
    }

    int cut_get_file_size_named_0(cut_u8arrview name0, cut_u64 *size)
    {
        Cut_File file;

        if (!cut_open_file_0(&file, name0, 0, 1))
            return 0;

        int get_size_success = cut_get_file_size(file, size);

        cut_close_file(file);

        return get_size_success;
    }

    int cut_read_file(Cut_File file, cut_u64 size, cut_u8Arrview *out, Cut_Mem_Allocator *mem_allocator)
    {
        out->data = cut_mem_allocate(sizeof(cut_u8) * size, mem_allocator);
        if (!cut_read_file2(file, size, out)) {
            cut_mem_free(out->data, mem_allocator);
            return 0;
        }
        return 1;
    }

    int cut_read_file2(Cut_File file, cut_u64 size, cut_u8Arrview *out)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_u64 total_read = 0;
            cut_u32 read;
            while (total_read < size) {
                if (!cut_WIN32_ReadFile(file.handle, out->data + total_read, size - total_read, &read, 0))
                    return 0;
                total_read += read;
            }
            out->count = total_read;
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            cut_u64 total_read = 0;
            ssize_t rd;
            while (total_read < size) {
                rd = read(file.handle, out->data + total_read, size - total_read);
                if (rd == -1)
                    return 0;
                total_read += rd;
            }
            out->count = total_read;
        }
        #endif

        return 1;
    }

    static inline int cut_read_entire_file_(Cut_File file, cut_u8Arrview *out, cut_u64 file_size)
    {
        cut_u64 prev_pos;
        if (!cut_get_file_position(file, &prev_pos))
            return 0;

        int res = cut_read_file2(file, file_size, out);

        cut_set_file_position(file, prev_pos);

        return res;
    }

    int cut_read_entire_file(Cut_File file, cut_u8arrview *out, Cut_Mem_Allocator *mem_allocator)
    {
        cut_u64 file_size;
        if (!cut_get_file_size(file, &file_size))
            return 0;

        out->data = cut_mem_allocate(sizeof(cut_u8) * file_size, mem_allocator);
        if (!cut_read_entire_file_(file, out, file_size)) {
            cut_mem_free(out->data, mem_allocator);
            return 0;
        }

        return 1;
    }

    int cut_read_entire_file2(Cut_File file, cut_u8Arrview *out)
    {
        cut_u64 file_size;
        if (!cut_get_file_size(file, &file_size))
            return 0;
        return cut_read_entire_file_(file, out, file_size);
    }

    int cut_read_entire_file_named(cut_u8arrview name, cut_u8arrview *out, Cut_Mem_Allocator *mem_allocator)
    {
        Cut_File file;

        if (!cut_open_file(&file, name, 0, 1))
            return 0;

        int read_success = cut_read_entire_file(file, out, mem_allocator);

        cut_close_file(file);

        return read_success;
    }

    int cut_read_entire_file_named_0(cut_u8arrview name0, cut_u8arrview *out, Cut_Mem_Allocator *mem_allocator)
    {
        Cut_File file;

        if (!cut_open_file_0(&file, name0, 0, 1))
            return 0;

        int read_success = cut_read_entire_file(file, out, mem_allocator);

        cut_close_file(file);

        return read_success;
    }

    int cut_write_file(Cut_File file, Cut_u8Arrview data)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_u32 total_write = 0;
            cut_u32 write;
            while (total_write < data.count) {
                if (!cut_WIN32_WriteFile(file.handle, data.data + total_write, data.count - total_write, &write, 0)) {
                    return 0;
                }
                total_write += write;
            }

            return 1;
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            cut_u64 total_write = 0;
            ssize_t written;
            while (total_write < data.count) {
                written = write(file.handle, data.data + total_write, data.count - total_write);
                if (written == -1)
                    return 0;
                total_write += written;
            }
            return 1;
        }
        #endif
    }

    int cut_write_file_named(Cut_u8Arrview name, Cut_u8Arrview data)
    {
        Cut_File file;

        if (!cut_open_file(&file, name, 1, 0))
            return 0;

        int write_success = cut_write_file(file, data);

        cut_close_file(file);

        return write_success;
    }

    int cut_write_file_named_0(Cut_u8Arrview name0, Cut_u8Arrview data)
    {
        Cut_File file;

        if (!cut_open_file_0(&file, name0, 1, 0))
            return 0;

        int write_success = cut_write_file(file, data);

        cut_close_file(file);

        return write_success;
    }
#endif
