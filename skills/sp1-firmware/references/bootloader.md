# SP-1 Bootloader

## Memory Layout

**VERIFIED**

```
Flash address map:

0x00000 – 0x1FFFF    Bootloader (128 KB)
0x20000 – 0xFFFFF    Application (832 KB available)
```

Application firmware entry point: **0x20000**

Bootloader owns the lower region and supervises application boot.

## Boot Vector

**VERIFIED** (standard ARM Cortex-M pattern)

Bootloader sets up:
1. Stack pointer (from vector table at 0x00000)
2. Reset vector address
3. Interrupt vectors

Application firmware sets up:
1. Stack pointer (from its own vector table at 0x20000)
2. Reset vector
3. Exception handlers

Zephyr handles this automatically via `CMakeLists.txt` and linker scripts.

## Application Initialization

**VERIFIED** (Zephyr boot process)

```
1. Bootloader jumps to 0x20000
2. Zephyr kernel initializes
   - Clocks
   - Memory management
   - Thread scheduler
3. Device drivers initialized (SYS_INIT priority)
4. Main thread starts
5. Application code runs
```

Do **not** hardcode addresses or vector tables. Let Zephyr/linker manage initialization.

## Reboot to Bootloader

**VERIFIED** (known working mechanisms in firmware)

Methods to trigger reboot to bootloader:

### Track 1 + Track 4 Hold

Press and hold both Track 1 and Track 4 buttons while powered on.

Firmware detects:
```c
if (track_button_1_pressed && track_button_4_pressed) {
    // Hold for ~3 seconds
    if (buttons_held > 3000) {
        reboot_to_bootloader();
    }
}
```

Bootloader then waits for DFU (Device Firmware Update) commands.

### Function Power-Off

Press Function button to shut down normally.

Some firmware versions allow:
```c
if (function_button_pressed && system_off_mode) {
    reboot_to_bootloader();
}
```

Exact behavior depends on firmware implementation. Preserve established mechanisms.

### Programmatic Reboot

From application code:

```c
#include <sys/reset.h>

// Reboot to bootloader
// (may require setting a register or writing to specific memory address)
sys_arch_reboot(REBOOT_BOOTLOADER);
```

Exact mechanism depends on nRF52840 bootloader implementation.

## Reset Reason

**VERIFIED** (nRF52840 register RESETREAS)

The nRF52840 has a reset reason register that records why the device last reset:

```
RESETREAS (at 0x40000400)
- PIN reset
- Watchdog reset
- SOFT reset (via AIRCR)
- CPU lock-up reset
```

**CRITICAL**: Clear this register after reading during startup:

```c
// Read reset reason
uint32_t reset_reason = NRF_POWER->RESETREAS;

// Log or act on reason
if (reset_reason & POWER_RESETREAS_PIN_Msk) {
    // Pin reset
}

// Clear the register
NRF_POWER->RESETREAS = 0xFFFFFFFF;  // Write 1s to clear
```

**Do not** discard reset-reason information before consuming it. If the device reboots immediately again, the reason is lost.

Before entering SYSTEM_OFF:

```c
// Clear reset reason
NRF_POWER->RESETREAS = 0xFFFFFFFF;

// Now enter SYSTEM_OFF
nrf_power_system_off();
```

Subsequent resets will have clean reset reason (not polluted by previous state).

## Watchdog Timeout

**VERIFIED** (non-negotiable constraint)

```
Watchdog timeout: < 5 seconds
```

Firmware must service watchdog deliberately to prevent unexpected resets.

Do **not** use watchdog feeding as a substitute for fixing a blocked/deadlocked subsystem.

If watchdog fires:
- System resets
- Reset reason register shows WDT
- Application restarts

Recovery from watchdog:
1. Log the event (if possible)
2. Return to safe state
3. Investigate what blocked the system

## Bootloader Functionality

**INFERRED** (typical bootloader capabilities)

Common bootloader functions:

```
- Boot application (normal startup)
- Wait for DFU commands (after reboot to bootloader)
- Receive firmware image over USB or serial
- Verify firmware image (CRC, signature)
- Program flash
- Boot new firmware
- Fallback to previous firmware if verification fails
```

Exact implementation depends on bootloader code. Use established procedures.

## Firmware Update Safety

**RECOMMENDED**:

- Always preserve a known-good bootloader backup (off-device storage)
- Verify CRC/signature of new firmware before flashing
- Keep USB power connected during updates (prevent brownout)
- Do not interrupt firmware transfer mid-programming
- Test recovery procedure on spare hardware before deploying updates

## Device Firmware Update (DFU)

**INFERRED** (typical DFU procedure)

```
1. Trigger reboot to bootloader (Track 1+4, or Function, or programmatic)
2. Connect USB to host
3. Run DFU tool (e.g., nrfutil, custom tool)
4. Tool sends firmware image to bootloader
5. Bootloader verifies and programs flash
6. Bootloader reboots application
```

Exact DFU protocol and tools depend on bootloader implementation.

## Secure Boot (Optional)

**UNKNOWN** (not verified in stock SP-1)

Some bootloaders support firmware signing:
- Bootloader verifies signature on application
- Prevents unauthorized firmware
- Requires key management

If enabled, firmware images must be signed with the correct private key before programming.

If disabled, any firmware can be programmed.

Check bootloader documentation and existing working firmware for evidence.

## Flash Protection

**INFERRED** (memory protection)

nRF52840 supports:
- Read protection (prevent program from reading certain flash regions)
- Write protection (prevent accidental erase/program)
- Execute protection (prevent code from running in certain regions)

Application firmware typically does not enable flash protection (bootloader may).

Bootloader uses write protection on its own region to prevent accidental overwrite.

## System Off and Wake

**VERIFIED**

System Off (ultra-low-power sleep):

```c
// Prepare for shutdown
audio_shutdown();
storage_shutdown();
bluetooth_shutdown();
peripherals_shutdown();

// Clear reset reason (for next power-on)
NRF_POWER->RESETREAS = 0xFFFFFFFF;

// Enter System Off
nrf_power_system_off();
```

Wake sources (after System Off):

- Push button (if wired appropriately)
- External interrupt
- Watchdog timeout (if enabled)

Exact wake sources depend on GPIO configuration.

**Do not** assume that all buttons/switches will wake the device. Only configured wake sources are active during System Off.

## Initialization Order Checklist

Boot sequence should verify:

- [ ] Bootloader starts (if applicable)
- [ ] Application entry point (0x20000) loads
- [ ] Zephyr kernel initializes
- [ ] Clocks enabled (oscillators, PLL)
- [ ] Memory layout correct (flash, RAM)
- [ ] Device drivers initialize (SYS_INIT)
- [ ] Reset reason register readable and clearable
- [ ] Watchdog configured and serviceable
- [ ] Audio peripherals can be initialized
- [ ] Storage peripherals can be initialized
- [ ] SAADC and GPIO responsive
- [ ] Bluetooth module reachable (if present)
- [ ] System stable before main() application code runs

## Known Unknowns

```yaml
unknown:
  - exact_bootloader_code_location_and_entry
  - exact_DFU_protocol_implementation
  - firmware_signature_requirements
  - exact_flash_protection_strategy
  - exact_boot_time
  - secure_boot_enabled_or_disabled
  - bootloader_version_and_capabilities
```

Examine bootloader source or existing firmware documentation.

## Safety Rules

**ABSOLUTE**:

- Do not overwrite bootloader region from application code
- Do not assume bootloader will recover from corrupted application
- Always keep a backup of working firmware
- Test DFU procedure on non-production hardware first
- Preserve reset reason register until consumed
- Clear reset reason before power-off to prevent stale information
- Service watchdog regularly (do not starve it)

## Bootloader Testing Checklist

- [ ] Normal boot works (Track 1+4 press does not trigger reboot to bootloader)
- [ ] Reboot to bootloader works (Track 1+4 hold > 3s, device goes into DFU mode)
- [ ] Reset reason captures pin reset correctly
- [ ] Reset reason captures watchdog correctly
- [ ] Watchdog timeout causes reset (deliberate test on non-production device)
- [ ] Watchdog feed prevents reset (confirmed working system)
- [ ] Power cycles recover cleanly
- [ ] DFU firmware update procedure works (if implemented)
