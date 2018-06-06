# WLib TLSF

Two-level segregated-fit memory allocator package for WLib. Allocator created by Matthew Conte.

Modified API for WLib.

# API Usage

The documentation is a little lacking on initialization but you essentially
pass a memory pool for TLSF to manage.

```c++
enum { STATIC_POOL_SIZE = 1 << 20 }; 
static char s_pool[STATIC_POOL_SIZE];

...
tlsf_t instance = tlsf_create_with_pool(s_pool, STATIC_POOL_SIZE);
```
