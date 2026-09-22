/**
 * sp1_api.h — Public developer API for SP-1 firmware
 * Modules program against this header only.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifndef SP1_HOST_TEST
#include <zephyr/sys/iterable_sections.h>
#endif

/* ---- Audio ---- */
typedef struct { int16_t l, r; } sp1_sample_t;

typedef void (*sp1_process_fn)(void *ctx,
                               const sp1_sample_t *in,
                               sp1_sample_t *out,
                               size_t frames);

struct sp1_module {
    sp1_process_fn process;
    void *ctx;
    const char *name;
};

#ifdef SP1_HOST_TEST
struct sp1_module;
extern struct sp1_module *sp1_test_modules[];
extern size_t sp1_test_module_count;
void sp1_test_register_module(struct sp1_module *module);
#define SP1_MODULE(_name, _process, _ctx) \
    static struct sp1_module _name = { \
        .process = (_process), .ctx = (_ctx), .name = #_name \
    }; \
    static void __attribute__((constructor)) _name##_register(void) \
    { \
        sp1_test_register_module(&_name); \
    }
#else
#define SP1_MODULE(_name, _process, _ctx) \
    STRUCT_SECTION_ITERABLE(sp1_module, _name) = { \
        .process = (_process), .ctx = (_ctx), .name = #_name \
    }
#endif

size_t   sp1_audio_block_size(void);
uint32_t sp1_sample_rate(void);

/* ---- Controls ---- */
typedef enum {
    SP1_FADER_1 = 0, SP1_FADER_2, SP1_FADER_3, SP1_FADER_4, SP1_FADER_COUNT
} sp1_fader_t;

typedef enum {
    SP1_BTN_TRACK_1 = 0, SP1_BTN_TRACK_2, SP1_BTN_TRACK_3, SP1_BTN_TRACK_4,
    SP1_BTN_PLAY, SP1_BTN_FUNC, SP1_BTN_VOL_UP, SP1_BTN_VOL_DOWN,
    SP1_BTN_FWD, SP1_BTN_RWD, SP1_BTN_COUNT
} sp1_btn_t;

float sp1_fader(sp1_fader_t f);
bool  sp1_btn_down(sp1_btn_t b);
bool  sp1_btn_pressed(sp1_btn_t b);
bool  sp1_btn_released(sp1_btn_t b);

/* ---- LEDs ---- */
void sp1_led_set(uint8_t led, uint8_t brightness);
void sp1_led_track(uint8_t track, bool on);
void sp1_led_playback(uint8_t idx, bool on);
void sp1_led_all_off(void);

/* ---- MIDI ---- */
typedef enum { SP1_MIDI_USB = 0, SP1_MIDI_BLE, SP1_MIDI_BOTH } sp1_midi_transport_t;

void sp1_midi_note_on(uint8_t ch, uint8_t note, uint8_t vel);
void sp1_midi_note_off(uint8_t ch, uint8_t note);
void sp1_midi_cc(uint8_t ch, uint8_t cc, uint8_t val);
void sp1_midi_clock(void);
void sp1_midi_start(void);
void sp1_midi_stop(void);

typedef void (*sp1_midi_rx_cb)(uint8_t status, uint8_t d1, uint8_t d2, void *user);
void sp1_midi_on_rx(sp1_midi_rx_cb cb, void *user);
void sp1_midi_set_transport(sp1_midi_transport_t t);
void sp1_midi_dispatch_rx(uint8_t status, uint8_t d1, uint8_t d2);

/* ---- System ---- */
uint32_t sp1_millis(void);
void     sp1_request_reboot_to_bootloader(void);
void     controls_scan(void);   /* call from main loop */
void     system_feed_watchdog(void);
void     audio_process_block(sp1_sample_t *in, sp1_sample_t *out, size_t frames);
