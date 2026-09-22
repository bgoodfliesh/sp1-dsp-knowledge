#ifndef SP1_TEST_H
#define SP1_TEST_H

#include <stdint.h>
#include <stdio.h>

typedef void (*test_fn)(void);

struct test_case {
    const char *name;
    test_fn run;
};

extern unsigned test_checks;
extern unsigned test_failures;
int write_wav_s16(const char *path, const int16_t *left,
                  const int16_t *right, size_t frames);

#define TEST_ASSERT(condition) do { \
    test_checks++; \
    if (!(condition)) { \
        fprintf(stderr, "    assertion failed: %s (%s:%d)\n", \
                #condition, __FILE__, __LINE__); \
        test_failures++; \
    } \
} while (0)

#define TEST_ASSERT_EQ_INT(expected, actual) do { \
    long long test_expected = (long long)(expected); \
    long long test_actual = (long long)(actual); \
    test_checks++; \
    if (test_expected != test_actual) { \
        fprintf(stderr, "    expected %lld, got %lld (%s:%d)\n", \
                test_expected, test_actual, __FILE__, __LINE__); \
        test_failures++; \
    } \
} while (0)

#define TEST_ASSERT_NEAR(expected, actual, tolerance) do { \
    double test_expected = (double)(expected); \
    double test_actual = (double)(actual); \
    double test_tolerance = (double)(tolerance); \
    test_checks++; \
    if ((test_actual < test_expected - test_tolerance) || \
        (test_actual > test_expected + test_tolerance)) { \
        fprintf(stderr, "    expected %.6f +/- %.6f, got %.6f (%s:%d)\n", \
                test_expected, test_tolerance, test_actual, \
                __FILE__, __LINE__); \
        test_failures++; \
    } \
} while (0)

#endif
