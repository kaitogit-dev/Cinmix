# Cinmix 0.14

Cinmix is a freestanding 32-bit x86 desktop operating system, booted from a legacy BIOS El Torito ISO. The preferred display is now **1920 × 1080 at 32-bit color**, with a Windows 7 Aero / Windows 11-inspired interface, centered icon-only taskbar, anti-aliased controls, and animated windows.

## New in 0.14

- Prefer **1920 × 1080 × 32** through VBE mode discovery or Bochs-compatible graphics registers. The 1080p framebuffer pitch is **7680 bytes per row**. A lower VBE resolution or VGA fallback remains available if the emulator does not support full HD; the OS does not falsely stretch lower-resolution modes.
- Lifted the renderer's old 1280 × 900 ceiling and expanded the default app windows to 1050 × 700 at full HD. The native regression test allocates both 1920 × 1080 backbuffers and checks exact framebuffer copies, controls and partial redraws.
- **120 Hz animation scheduler** in place of 60 Hz. Windows now open in three eased steps (about 25 ms in ideal timing), maximize/snap in four steps, and close/minimize in five steps. Panel tweens use six steps (about 50 ms). Real timing still depends on emulator rendering speed.
- Keep the taskbar hover delay at approximately **one second** by adjusting it to 120 animation-clock pulses.
- Window rendering now avoids large, unnecessary translucent blends behind opaque window interiors, concentrating those glass passes on edges and title bars to reduce cost at 1080p.
- Preserved the cursor, active/hover taskbar glass highlights, minimized and closing window tweens, Alt+Tab, Quick Panel, Notification Center, desktop icons, and existing applications.

## Existing features

Ten built-in apps, a searchable Start menu, draggable and resizable windows, minimize, close, maximize, snap, animated Show Desktop, right-click desktop context menu, reversible open/close panels, keyboard desktop focus, visual Alt+Tab, translucent themes, wallpaper and accent settings, Run dialog (`Ctrl+R`), and delayed taskbar previews.

## Build

Requires GCC, GNU binutils and Python 3 on Linux, including `-m32` compilation support.

```sh
make verify
```

The resulting image is `Cinmix-0.14.iso`. `make verify` checks the El Torito image and bootloader, validates the 1920 × 1080 mode-selection instructions, and runs native regression suites at 720p as well as a real 1080p framebuffer test.

### Testing limitation

The full-resolution compositor, event handling, and ISO structure have been tested natively. **An actual BIOS emulator boot has not been tested in this environment.** The emulator must support a 1920 × 1080 × 32 graphics mode and enough video memory (roughly 8 MiB for one framebuffer), or the firmware will select a supported fallback. The emulator also needs sufficient system RAM for two 1080p software buffers and the OS.

The software volume interface does not yet output to an audio device driver. Font licensing is in `FONT-LICENSE.txt`.
