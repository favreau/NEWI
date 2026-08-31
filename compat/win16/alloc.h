#ifndef NEWI_COMPAT_ALLOC_H
#define NEWI_COMPAT_ALLOC_H

// Borland <alloc.h>. The far/huge heap variants collapse onto the flat heap.

#include <cstdlib>

#ifndef __BORLANDC__
inline void *farmalloc(unsigned long size) { return malloc((size_t)size); }
inline void farfree(void *block) { free(block); }
inline void *farcalloc(unsigned long count, unsigned long size)
{
  return calloc((size_t)count, (size_t)size);
}
inline void *farrealloc(void *block, unsigned long size)
{
  return realloc(block, (size_t)size);
}
#endif

#endif // NEWI_COMPAT_ALLOC_H
