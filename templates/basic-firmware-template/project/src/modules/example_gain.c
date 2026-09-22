#include "sp1_api.h"

static struct { float gain; } state;

static void process(void *ctx, const sp1_sample_t *in,
                    sp1_sample_t *out, size_t frames)
{
    state.gain = sp1_fader(SP1_FADER_1);
    for (size_t i = 0; i < frames; i++) {
        out[i].l = (int16_t)(in ? in[i].l * state.gain : 0);
        out[i].r = (int16_t)(in ? in[i].r * state.gain : 0);
    }
}

SP1_MODULE(example_gain, process, &state);
