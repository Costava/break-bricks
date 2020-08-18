#include "rand.h"

#include <stdlib.h>

double rand_double01() {
	return ((double)rand()) / ((double)RAND_MAX);
}

double rand_double(const double min, const double max) {
	const double range = max - min;

	return min + (rand_double01() * range);
}

int rand_int(const int min, const int max) {
	const int range = max - min;

	return min + (rand() % (range + 1));
}
