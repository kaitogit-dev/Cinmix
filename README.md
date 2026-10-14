# Cinmix 0.11.3

Cinmix is a freestanding 32-bit x86 desktop OS booting from a legacy BIOS El Torito ISO. The UI targets 1280 x 720 and combines Aero-style glass with a centered icon-only taskbar.

## What's included

Ten desktop apps, a searchable Start menu, draggable and resizable windows, maximize/restore, minimize/close, window snapping, tweened transitions, notification and quick panels, live taskbar previews, desktop context menus, themes, wallpaper and accent controls, and anti-aliased rounded edges.

## 0.11.3 repair

- Replaced the emulator-sensitive PC-speaker animation pulse with a polled PIT channel 0 counter and a fallback when timer samples stop changing.
- Window controls accept clicks at both the visible tweened position and the final position. Minimize, restore and close transitions preserve their state.
- Animation-only frames use damaged regions rather than forcing a full-screen repaint. Idle windows no longer redraw unnecessarily.
- Preserved the compositor cursor correction, existing apps, notifications, appearance controls and anti-aliased corners.
- Added PS/2 mouse-packet regression coverage for minimize, restore, close and maximize.

## Build and test

On Linux with GCC, GNU binutils, Python 3 and an i386-capable GCC toolchain:

```sh
make verify
```

The output is `Cinmix-0.11.3.iso`. `make verify` checks the ISO boot structure and runs native UI/compositor regression tests. These native tests do not replace a full BIOS-emulator boot test; runtime performance and input behavior in v86 or QEMU should still be confirmed on the intended emulator.

The volume control is a software UI state, not an audio device driver. Font licensing information is in `FONT-LICENSE.txt`.
