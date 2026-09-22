#include "sp1_api.h"

/*
 * Transport-independent MIDI seam. The USB/BLE driver should call
 * midi_dispatch_rx() after it is wired to the board. Keeping registration
 * here lets plugins and host tests use the same callback contract.
 */
static sp1_midi_rx_cb rx_callback;
static void *rx_user;
static sp1_midi_transport_t transport = SP1_MIDI_USB;

__attribute__((weak)) void midi_hw_send(uint8_t status, uint8_t d1, uint8_t d2)
{
    (void)status;
    (void)d1;
    (void)d2;
}

void sp1_midi_dispatch_rx(uint8_t status, uint8_t d1, uint8_t d2)
{
    if (rx_callback)
        rx_callback(status, d1, d2, rx_user);
}

void sp1_midi_note_on(uint8_t ch, uint8_t note, uint8_t vel)
{
    midi_hw_send((uint8_t)(0x90u | (ch & 0x0fu)), note & 0x7fu, vel & 0x7fu);
}

void sp1_midi_note_off(uint8_t ch, uint8_t note)
{
    midi_hw_send((uint8_t)(0x80u | (ch & 0x0fu)), note & 0x7fu, 0u);
}

void sp1_midi_cc(uint8_t ch, uint8_t cc, uint8_t val)
{
    midi_hw_send((uint8_t)(0xb0u | (ch & 0x0fu)), cc & 0x7fu, val & 0x7fu);
}

void sp1_midi_clock(void) { midi_hw_send(0xf8u, 0u, 0u); }
void sp1_midi_start(void) { midi_hw_send(0xfau, 0u, 0u); }
void sp1_midi_stop(void)  { midi_hw_send(0xfcu, 0u, 0u); }

void sp1_midi_on_rx(sp1_midi_rx_cb cb, void *user)
{
    rx_callback = cb;
    rx_user = user;
}

void sp1_midi_set_transport(sp1_midi_transport_t t)
{
    if (t <= SP1_MIDI_BOTH)
        transport = t;
    (void)transport;
}
