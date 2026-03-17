/*
 Minimal shim providing the allocations global normally
 defined via START_MEMORY in viriatum.c, so that the Zig
 example can link without pulling in main().
*/

#include <stddef.h>

size_t allocations = 0;
