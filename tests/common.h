#include <voxflat.h>
#include <stdlib.h>
#include <stdbool.h>

#define ASSERT(cond) \
    do { if (!(cond)) { \
        fprintf(stderr, "Assertion failed: `%s` is not true (%s:%d)\n", #cond, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    }} while(false)

#define ASSERT_EQ(expected, expr) \
    do { if ((intmax_t)(expected) != (intmax_t)(expr)) { \
        fprintf(stderr, "Assertion failed: `%s` is %jd but %jd was expected (%s:%d)\n", #expr, (intmax_t)(expr), (intmax_t)(expected), __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    }} while(false)
