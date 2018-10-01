# WLib TLSF

Two-level segregated-fit memory allocator package for WLib. Allocator created by Matthew Conte.

Modified API for WLib.

# Install

```bash
wio install wlib-tlsf
```

# API Usage

The documentation is a little lacking on initialization but you essentially
pass a memory pool for TLSF to manage.

```c++
enum { STATIC_POOL_SIZE = 1 << 20 };
static char s_pool[STATIC_POOL_SIZE];

tlsf_t instance = tlsf_create_with_pool(s_pool, STATIC_POOL_SIZE);

void *data = tlsf_malloc(instance, 64);
tlsf_free(instance, data);
```

# Configuration
Three main flags are used to control the operation of the TLSF.

#### `WLIB_TLSF_LOG2_DIV`
Log of the number of subdivisions within a size class. Larger values
introduce more overhead to the TLSF structure but reduce fragmentation.
Values should be at least 2. Typical values up to 5.

#### `WLIB_TLSF_LOG2_ALIGN`
Log of the alignment of memory and blocks. E.g. a value of 2 aligns to
4 bytes. The alignment must be at least the architecture alignment;
that is, 2 bytes for 16-bit, 4 bytes for 32-bit, and 8 bytes for 64-bit.

Larger values may be used to align to page boundaries.

#### `WLIB_TLSF_LOG2_MAX`
Log of the maximum possible allocation size. This value directly affects
the maximum pool size that can be managed by TLSF. Increasing it,
however, will increase overhead.

#### Specify Architecture
One of `WLIB_TLSF_16BIT`, `WLIB_TLSF_32BIT`, or `WLIB_TLSF_64BIT` must
be specified to select the architecture. TLSF attempts to dectect the
specific architecture to use builtins, but these can be specified as

- `WLIB_TLSF_ARM`
- `WLIB_TLSF_GHS`
- `WLIB_TLSF_GNU`
- `WLIB_TLSF_MSC`
- `WLIB_TLSF_PPC`
- `WLIB_TLSF_SNC`

If none is specified or autodetection fails, TLSF falls back to a 
generic implementation of CLZ.

## Debugging
The flag `WLIB_TLSF_DEBUG_LEVEL` specifies the amount of printing and
run-time assertions. A value of `0` turns all off, a value of `1` will
print errors and run asserts, and a value of `2` will print some
verbose output.

Custom assert and print handlers can be implemented by defining
`WLIB_TLSF_ASSERT` and `WLIB_TLSF_PRINTF` and then providing the functions

```c
void tlsf_printf(const char *fmt, ...) 
{ /* ... */ }

void tlsf_assert(bool expr, const char *msg)
{ /* ... */ }
```

## Usage on AVR
Recommended settings are

```yaml
definitions:
  package:
  - -DWLIB_TLSF_16BIT
  - -DWLIB_TLSF_LOG2_DIV=2
  - -DWLIB_TLSF_LOG2_ALIGN=1
  - -DWLIB_TLSF_LOG2_MAX=10
  - -DWLIB_TLSF_PRINTF
  - -DWLIB_TLSF_ASSERT
```

For boards will limited RAM.
