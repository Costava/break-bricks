#ifndef CHARU_H
#define CHARU_H

// char utilities

#if defined(__linux__)
	// For `size_t`
	#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Copy `str` onto `start`
// `end` points to the last byte that is allowed to be written
// A NUL terminator is guaranteed to be written
// If `start`, `end`, or `str` is NULL, prints to stderr and exits
// If end comes before start, prints to stderr and exits
// Returns pointer to NUL terminator that was put in
char *charu_copy_until_end(char *start, char *const end, char *str);

// Concatenate `src` onto `dest`
// If `dest_size` is 0, nothing happens and the function returns `dest`
// Will write a NUL terminator (unless `dest_size` is 0)
// If `dest` or `src` is NULL, prints to stderr and exits
// Returns pointer to the NUL terminator that was written
char *charu_concat(char *dest, size_t dest_size, char *src);

#ifdef __cplusplus
}
#endif

#endif
