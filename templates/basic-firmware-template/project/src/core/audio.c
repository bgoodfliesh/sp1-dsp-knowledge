#include "sp1_api.h"
#ifndef SP1_HOST_TEST
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#endif
#include <string.h>

#define AUDIO_BLOCK_FRAMES  128
#define SAMPLE_RATE_HZ      48000

size_t   sp1_audio_block_size(void) { return AUDIO_BLOCK_FRAMES; }
uint32_t sp1_sample_rate(void)      { return SAMPLE_RATE_HZ; }

/* Placeholder — replace with real I2S + DMA callback */
void audio_process_block(sp1_sample_t *in, sp1_sample_t *out, size_t frames)
{
    memset(out, 0, frames * sizeof(*out));
#ifdef SP1_HOST_TEST
    for (size_t i = 0; i < sp1_test_module_count; i++) {
        struct sp1_module *m = sp1_test_modules[i];
#else
    STRUCT_SECTION_FOREACH(sp1_module, m) {
#endif
        if (m->process)
            m->process(m->ctx, in, out, frames);
    }
}

#ifndef SP1_HOST_TEST
static int audio_init(void)
{
    /* TODO: configure I2S as slave, CS42L42 master @ 48 kHz */
    return 0;
}

SYS_INIT(audio_init, APPLICATION, 50);
#endif
