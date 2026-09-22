#include "sp1_api.h"

static void test_gain_process(void *ctx, const sp1_sample_t *in,
                              sp1_sample_t *out, size_t frames)
{
    (void)ctx;
    float gain = sp1_fader(SP1_FADER_1);
    for (size_t i = 0; i < frames; i++) {
        out[i].l = (int16_t)(in[i].l * gain);
        out[i].r = (int16_t)(in[i].r * gain);
    }
}

SP1_MODULE(test_gain, test_gain_process, NULL);
