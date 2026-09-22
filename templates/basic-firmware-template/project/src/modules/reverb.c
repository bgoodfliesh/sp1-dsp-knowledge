/**
 * Thin SP1_MODULE wrapper around the Clouds-derived reverb.
 * Included in the project as a reusable reference; linked when the user
 * selects the Reverb DSP option.
 */
#include "sp1_api.h"
#include "sp1_dsp_reverb.h"

static int16_t reverb_line[4608];
static Reverb  rv;

static struct {
    int32_t krt;   /* feedback, q8 */
    int32_t klp;   /* damping, q8 */
    int32_t wet;   /* mix, q8 */
} state = {
    .krt = 180,
    .klp = 200,
    .wet = 128,
};

static void process(void *ctx, const sp1_sample_t *in,
                    sp1_sample_t *out, size_t frames)
{
    (void)ctx;

    /* Map faders → parameters (example mapping) */
    state.krt = (int32_t)(sp1_fader(SP1_FADER_1) * 255.f);
    state.klp = (int32_t)(sp1_fader(SP1_FADER_2) * 255.f);
    state.wet = (int32_t)(sp1_fader(SP1_FADER_3) * 255.f);

    /* Convert to int32 working buffers */
    int32_t mixL[256], mixR[256];
    if (frames > 256) frames = 256;          /* safety */

    for (size_t i = 0; i < frames; i++) {
        mixL[i] = in ? in[i].l : 0;
        mixR[i] = in ? in[i].r : 0;
    }

    if (!rv.live)
        reverb_reset(&rv);

    reverb_process_block(&rv, mixL, mixR, (uint32_t)frames,
                         state.krt, state.klp, state.wet);

    for (size_t i = 0; i < frames; i++) {
        out[i].l = (int16_t)(mixL[i] > 32767 ? 32767 :
                             mixL[i] < -32768 ? -32768 : mixL[i]);
        out[i].r = (int16_t)(mixR[i] > 32767 ? 32767 :
                             mixR[i] < -32768 ? -32768 : mixR[i]);
    }
}

static int reverb_module_init(void)
{
    reverb_bind(&rv, reverb_line, 4608, NULL);
    reverb_reset(&rv);
    return 0;
}
SYS_INIT(reverb_module_init, APPLICATION, 60);

SP1_MODULE(reverb, process, &state);
