#ifndef EASY_ALLOC_H
#define EASY_ALLOC_H

#if defined(__linux__)
	// For `size_t`
	#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Do not try to alloc/realloc a size of 0
//  because malloc/realloc will return NULL and these functions will call `exit`
//  ?
void *easy_malloc(size_t size);
void *easy_realloc(void *ptr, size_t new_size);

#ifdef __cplusplus
}
#endif

#endif
