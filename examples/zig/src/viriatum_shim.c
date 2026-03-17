/*
 Minimal shim providing the allocations global normally
 defined via START_MEMORY in viriatum.c, so that the Zig
 example can link without pulling in main().

 In debug builds allocations starts at 0 (tracking enabled),
 in release builds it starts at -1 (tracking disabled),
 matching the START_MEMORY macro in memory.h.
*/

#include <stddef.h>

#ifdef VIRIATUM_DEBUG
size_t allocations = 0;
#else
size_t allocations = -1;
#endif
