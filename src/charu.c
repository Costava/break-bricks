#include "charu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *charu_copy_until_end(char *start, char *const end, char *str) {
	if (start == NULL) {
		fprintf(stderr, "%s: start must not be NULL\n", __func__);
		exit(EXIT_FAILURE);
	}

	if (end == NULL) {
		fprintf(stderr, "%s: end must not be NULL\n", __func__);
		exit(EXIT_FAILURE);
	}

	if (str == NULL) {
		fprintf(stderr, "%s: str must not be NULL\n", __func__);
		exit(EXIT_FAILURE);
	}

	if (end < start) {
		fprintf(stderr, "%s: end must not come before start\n", __func__);
		exit(EXIT_FAILURE);
	}

	while (start < end && str[0] != '\0') {
		start[0] = str[0];
		start += 1;
		str += 1;
	}

	start[0] = '\0';

	return start;
}

char *charu_concat(char *dest, size_t dest_size, char *src) {
	if (dest_size == 0) {
		return dest;
	}

	const int deststrlen = strlen(dest);

	char *start = dest + deststrlen;
	char *const end = dest + dest_size - 1;

	return charu_copy_until_end(start, end, src);
}
