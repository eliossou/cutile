# cutile - C Utility Library

**cutile** is a collection of single-header C utility modules designed for simplicity, portability, and ease of integration. Each module is a `.c` file that can be included directly, no build system required.

## Usage

Include any module in **exactly one** translation unit with `CUT_IMPL` defined (or the module-specific `CUT_<MODULE>_IMPL` define) to generate its implementation. Include it in other files without the define to get only declarations.

```c
#define CUT_IMPL
#include "cutile/memory.c"
#include "cutile/print.c"
```

Short names (e.g., `mem_cpy` instead of `cut_mem_cpy`) are available by defining `CUT_SHORT_NAMES` or the module-specific `CUT_<MODULE>_SHORT_NAMES`.

## Modules

| Module | File | Description |
|---|---|---|
| **Base** | `base.c` | Fundamental types (`cut_u8`–`cut_u64`, `cut_uptrsize`), pointer-size detection, assertions, helpers (`cut_inlinable`, `cut_internal`, `cut_array_size`, `cut_align_nb`, `cut_MIN`/`cut_MAX`) |
| **Memory** | `memory.c` | Memory copy/compare/set, pluggable allocator (`Cut_Mem_Allocator`), fixed-buffer allocator, virtual-memory allocator (VirtualAlloc/mmap), circular allocator |
| **Print** | `print.c` | Formatted printing (`cut_print`, `cut_sprint`), formatters for ints (decimal/hex), strings, and string literals |
| **Str** | `str.c` | Null-terminated string utilities: length, equality, conversion to `cut_u8Arrview` |
| **Dyn Array** | `dyn_array.c` | Generic dynamic array with element-level growth, back-insertion, iteration, removal |
| **Hash Table** | `hash_table.c` | Open-addressing hash table with FNV-32 hashing, linear probing, support for custom key types |
| **File** | `file.c` | Cross-platform file I/O: open/close, read/write, file size, entire-file reading (Windows via `CreateFile`, Linux/macOS via POSIX) |
| **INI** | `ini.c` | INI file parser with section/field tokenizer, global and section-scoped value lookups |
| **UTF-8** | `utf8.c` | UTF-8 character length, codepoint decoding, string-length in characters, iteration macro |
| **Stacktrace** | `stacktrace.c` | Cross-platform stack trace capture (requires platform-specific init) |
| **Memory Debug** | `memory_debug.c` | Debug allocator wrapping a real allocator with allocation tracking, stack traces, and invalid-free detection |
| **Win32 (internal)** | `win32.c` | Thin type-safe wrapper around Windows API symbols used internally by other modules |

## License

Public domain — see `license.txt`.
