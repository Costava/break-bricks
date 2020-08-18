#include "easy_alloc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// Based on
// https://stackoverflow.com/questions/35026910/malloc-error-checking-methods
void *easy_malloc(size_t size) {
	void *ptr = malloc(size);

	if (ptr == NULL) {
		fprintf(stderr, "%s: Failed to malloc %zu bytes\n", __func__, size);

		exit(EXIT_FAILURE);
	}

	return ptr;
}

void *easy_realloc(void *ptr, size_t new_size) {
	ptr = realloc(ptr, new_size);

	if (ptr == NULL) {
		fprintf(stderr, "%s: Failed to realloc to %zu bytes\n",
			__func__, new_size);

		exit(EXIT_FAILURE);
	}

	return ptr;
}
