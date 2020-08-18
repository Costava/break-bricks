#ifndef RAND_H
#define RAND_H

#ifdef __cplusplus
extern "C" {
#endif

// Return a random double in range [0, 1] (both inclusive)
double rand_double01();

// Return a random double in range [min, max]
// Max should be greater than min
double rand_double(const double min, const double max);

// Return a random int in range [min, max] (both inclusive)
int rand_int(const int min, const int max);

#ifdef __cplusplus
}
#endif

#endif
