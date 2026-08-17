# cutile - C Utility Library

**cutile** is a collection of single-header C utility modules designed for simplicity, portability, and ease of integration. Each module is a `.c` file that can be included directly, no build system required.

## Usage

Include any module in **exactly one** translation unit with `CUT_IMPL` defined (or the module-specific `CUT_<MODULE>_IMPL` define) to generate its implementation. Include it in other files without the define to get only declarations.

```c
#define CUT_IMPL
#include "cutile/memory.c"
#include "cutile/print.c"
```

Short names (e.g., `Arena` instead of `Cut_Arena`) are available by defining `CUT_SHORT_NAMES` or the module-specific `CUT_<MODULE>_SHORT_NAMES`.

## Modules

| Module | File | Description |
|---|---|---|
| **Base** | `base.c` | Fundamental types (`cut_u8`–`cut_u64`, `cut_uptrsize`), pointer-size detection, assertions, helpers (`cut_inlinable`, `cut_internal`, `cut_array_size`, `cut_align_nb`, `cut_MIN`/`cut_MAX`), etc |
| **Memory** | `memory.c` | Pluggable allocator (`Cut_Mem_Allocator`), arena allocators, circular allocator... |
| **Print** | `print.c` | Formatted printing (`cut_print`, `cut_sprint`), formatters for ints (decimal/hex), strings... |
| **Str** | `str.c` | Null-terminated string utilities: length, equality, conversion to `cut_u8Arrview`... |
| **Dyn Array** | `dyn_array.c` | Generic dynamic array with element-level growth, back-insertion, iteration, removal |
| **Hash Table** | `hash_table.c` | Open-addressing hash table with FNV-32 hashing, linear probing, support for custom key types |
| **File** | `file.c` | Cross-platform file I/O: open/close, read/write, file size, entire-file reading (Windows via `CreateFile`, Linux/macOS via POSIX) |
| **Network** | `network.c` | Cross-platform IPv4/IPv6 TCP and UDP sockets: open/close, blocking mode, endpoints, bind/connect/listen/accept, send/receive |
| **INI** | `ini.c` | INI file parser with section/field tokenizer, global and section-scoped value lookups |
| **UTF-8** | `utf8.c` | UTF-8 character length, codepoint decoding, string-length in characters, iteration macro |
| **Stacktrace** | `stacktrace.c` | Cross-platform stack trace capture |
| **Memory Debug** | `memory_debug.c` | Debug allocator wrapping a real allocator with allocation tracking, stack traces, and invalid-free detection |
| **Preprocessor** | `pp.c` | Preprocessor utilities |

## License

Public domain — see `license.txt`.
