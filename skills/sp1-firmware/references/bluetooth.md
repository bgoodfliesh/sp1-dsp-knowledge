# SP-1 Bluetooth

## Bluetooth Module Hardware

**VERIFIED** (see hardware.md for detailed pins)

```
Device:     Infineon CYBT-353027-02
SoC:        CYW20706A2 (Cortex-M3)
Firmware:   Classic Bluetooth A2DP sink (stock)
Flash:      512 KB SPI (on module)
```

The Bluetooth module is **independent embedded hardware** with its own processor, radio, and flash.

### Communication Interface

**VERIFIED**

Communicates with nRF52840 via UART (HCI over H4):

```
Baud rate:  115200 (8-N-1)
Logic level: 1.8 V (from nRF52840 side)
Protocol:   HCI (Host Controller Interface)
Format:     H4 (HCI packet format with packet type indicator)
```

## Stock Bluetooth Firmware

**VERIFIED**

```
A2DP sink (audio streaming receiver)
- Receives stereo audio over Bluetooth
- Forwards to nRF52840 (HCI path)
- Does not expose BLE-MIDI
```

**INFERRED**: Stock firmware is read-only in normal operation. BLE-MIDI requires custom AIROC/WICED application.

## BLE-MIDI Service

**VERIFIED** (UUID from official MIDI-over-BLE specification)

Service UUID:
```
03B80E5A-EDE8-4B33-A751-6CE34EC4C700
```

I/O characteristic UUID:
```
7772E5DB-3868-4112-A1A9-F2669D106BF3
```

Current CCCD (Client Characteristic Configuration Descriptor) documented as **unencrypted**.

**Implementation note**: BLE-MIDI should be a transport backend abstraction. Public MIDI API should not care whether MIDI arrived from USB, BLE, or another source.

## HCI Command/Response

**VERIFIED** (HCI standard, H4 frame format)

Basic HCI operations:

```
HCI RESET:          opcode 0x0C03
    → Resets module state, closes connections
    → Use to recover from unresponsive state
    → Wait for response before next command

HCI HOST BUFFER SIZE: opcode 0x1A05
    → Set buffers for command/ACL data
    → Allows pipelining of commands

HCI VERSION INFO:    opcode 0x0401
    → Query firmware version, hardware version
    → Useful for validation
```

Each command must wait for HCI event response (or timeout) before issuing next command.

## Module Recovery Procedure

**VERIFIED** (known working sequence)

If Bluetooth module becomes unresponsive:

1. Hold CTS low (RTS/CTS flow control signal)
2. Pulse `RST_N` low for approximately 10 ms
3. Wait approximately 10 ms (let module boot)
4. Release CTS
5. Repeatedly issue HCI RESET until acknowledgement (with timeout)

After recovery:
- Module returns to default state
- Re-establish UART connection
- Reissue configuration commands
- Reopen connections if needed

**Never** include Bluetooth recovery in the audio execution path. Recovery is a control-plane operation, not audio-critical.

## Static Section Preservation

**CRITICAL**: Before flashing new Bluetooth firmware:

1. **Read** the existing Static Section (contains BD_ADDR, keys, RF calibration)
2. **Verify** it matches the expected factory template
3. **Preserve** it during firmware flash
4. **Restore** it after flashing new application

Static Section location and size depend on the specific firmware version. Refer to AIROC documentation.

Loss of Static Section may result in:
- Invalid Bluetooth address (bricked device)
- Loss of bonding/keys
- RF calibration reset (poor range/performance)

## Bluetooth Firmware Development Safety

**ABSOLUTE RULES**:

- **Never** use `CHIP_ERASE` during normal development
  - Use `Upgrade Download` mode when updating application firmware
  - `CHIP_ERASE` erases everything, including Static Section
  - Only acceptable on disposable evaluation boards

- **Always** use a **disposable evaluation board** for AIROC/WICED development
  - Production SP-1 hardware should never undergo risky firmware operations
  - If experimenting with custom BLE profiles, use spare hardware

- **Preserve** recovery paths
  - Keep a known-good bootloader backup
  - Document recovery procedures for new firmware versions
  - Test recovery sequence before deploying to production

## MIDI over Bluetooth (BLE-MIDI)

### Standard BLE-MIDI Structure

Standard BLE-MIDI (from official spec):

- GATT service for MIDI
- Characteristic for MIDI I/O
- Timestamp-based message framing
- Optional encryption

### SP-1 BLE-MIDI Implementation

**UNKNOWN** (custom AIROC firmware required)

To implement BLE-MIDI on SP-1:

1. Create custom AIROC application (CYW20706A2 firmware)
2. Expose MIDI service and characteristics
3. Implement timestamp and message framing
4. Bridge MIDI data to nRF52840 (HCI or custom protocol)
5. Validate on real hardware

### MIDI Transport Abstraction

**RECOMMENDED** (not yet verified in firmware)

Do **not** hard-wire BLE-MIDI into musical modules.

Instead, create a transport-independent MIDI layer:

```
MIDI transport abstraction
├── USB MIDI backend
├── BLE-MIDI backend
├── Other transports (MIDI In/Out connectors, serial, etc.)
└── Public MIDI API
        ├── Note On
        ├── Note Off
        ├── CC
        ├── Program Change
        ├── Clock
        ├── Start/Stop/Continue
        └── RX callback
```

Modules consume events from the public API, not directly from a physical transport.

## Zephyr Bluetooth Integration

**INFERRED** (depends on nRF52840 Bluetooth stack)

The nRF52840 can run Zephyr's own Bluetooth stack (for BLE peripheral), but the SP-1 uses external Bluetooth module (CYBT).

Zephyr does **not** directly control the SP-1 Bluetooth module (that is via HCI UART).

Zephyr can:
- Provide HCI transport (UART driver)
- Parse/format HCI packets
- Manage connection state
- Expose Bluetooth services (if applicable)

Zephyr does **not**:
- Provide the radio (external module provides)
- Control AIROC firmware (must be pre-flashed to module)

## Control-Plane vs Data-Plane

**RECOMMENDED** (separation of concerns)

- **Data-plane**: A2DP audio, BLE-MIDI I/O (real-time, low-latency)
- **Control-plane**: Connection management, service discovery, firmware updates (best-effort, can be deferred)

The audio path should **not** wait for Bluetooth control operations. Separate threads for control and audio.

If Bluetooth connection drops during playback:
- Audio continues (may mute or loop if no new A2DP frames)
- UI updates connection status
- Reconnection handled asynchronously

## Known Limitations

- Module firmware (stock) is proprietary; custom BLE-MIDI requires AIROC toolchain
- UART interface is single-threaded (one operation at a time; pipelined commands must wait for responses)
- Bluetooth range and performance depend on antenna design (module antenna is on-board; no external antenna port)
- Power consumption during A2DP playback is significant (keep USB power connected or ensure battery is charged)

## Unverified

```yaml
unknown:
  - exact_HCI_UART_latency
  - exact_BLE_connection_establishment_time
  - exact_A2DP_audio_latency
  - exact_power_consumption_during_A2DP
  - exact_recovery_time_after_module_reset
  - BLE_MIDI_implementation_details
  - custom_AIROC_firmware_build_procedure
  - Static_Section_structure_for_current_firmware_version
```

Document with source and measurement methodology.

## Bluetooth Checklist

When bringing up Bluetooth:

- [ ] UART configured (115200 8-N-1, 1.8 V logic)
- [ ] Module reset sequence working (RST_N pulse)
- [ ] HCI commands received (test with HCI RESET)
- [ ] Module responds with HCI events
- [ ] Connection can be established (pair/bond if applicable)
- [ ] Audio data received (A2DP) or MIDI data (if BLE-MIDI custom firmware)
- [ ] Control operations do not block audio path
- [ ] Power management (sleep/wake) functional
- [ ] Recovery procedure tested
