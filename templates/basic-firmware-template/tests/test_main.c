#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned test_checks;
unsigned test_failures;

extern const struct test_case reverb_tests[];
extern const size_t reverb_test_count;

static int run_suite(const char *name, const struct test_case *tests, size_t count)
{
    unsigned suite_failures = test_failures;

    printf("%s (%zu tests)\n", name, count);
    for (size_t i = 0; i < count; i++) {
        unsigned failures_before = test_failures;
        printf("  %-36s", tests[i].name);
        fflush(stdout);
        tests[i].run();
        if (test_failures == failures_before)
            puts("PASS");
        else
            puts("FAIL");
    }
    return test_failures != suite_failures;
}

int write_wav_s16(const char *path, const int16_t *left,
                  const int16_t *right, size_t frames)
{
    FILE *file = fopen(path, "wb");
    if (!file) {
        perror(path);
        return 1;
    }

    uint32_t data_bytes = (uint32_t)(frames * 4u);
    uint32_t riff_size = 36u + data_bytes;
    uint32_t format_size = 16u;
    uint16_t format = 1u;
    uint16_t channels = 2u;
    uint32_t sample_rate = 48000u;
    uint32_t byte_rate = sample_rate * 4u;
    uint16_t block_align = 4u;
    uint16_t bits_per_sample = 16u;

    fwrite("RIFF", 1, 4, file);
    fwrite(&riff_size, 4, 1, file);
    fwrite("WAVEfmt ", 1, 8, file);
    fwrite(&format_size, 4, 1, file);
    fwrite(&format, 2, 1, file);
    fwrite(&channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);
    fwrite("data", 1, 4, file);
    fwrite(&data_bytes, 4, 1, file);

    for (size_t i = 0; i < frames; i++) {
        fwrite(&left[i], sizeof(left[i]), 1, file);
        fwrite(&right[i], sizeof(right[i]), 1, file);
    }

    if (fclose(file) != 0) {
        perror(path);
        return 1;
    }
    printf("Wrote %s (%zu frames)\n", path, frames);
    return 0;
}

int main(int argc, char **argv)
{
    const char *wav_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--wav") == 0 && i + 1 < argc)
            wav_path = argv[++i];
        else {
            fprintf(stderr, "usage: %s [--wav path]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    int result = run_suite("SP-1 host tests", reverb_tests, reverb_test_count);
    printf("\n%u checks, %u failures\n", test_checks, test_failures);

    if (result || !wav_path)
        return result ? EXIT_FAILURE : EXIT_SUCCESS;

    extern int render_reverb_wav(const char *path);
    return render_reverb_wav(wav_path) ? EXIT_FAILURE : EXIT_SUCCESS;
}
