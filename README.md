# Cinmix 0.12

Cinmix is a freestanding 32-bit x86 desktop operating system designed for a legacy BIOS El Torito CD image at 1280 x 720. Its desktop combines Aero-style glass, a centered icon-only taskbar, anti-aliased edges, and animated windows.

## New in 0.12

- Removed translucent backing boxes from desktop shortcut icons and taskbar app icons. Individual icon artwork is drawn directly on the desktop or taskbar, without a generic plate.
- Kept small underlines for active/running taskbar apps, and replaced the boxed hover effect with a discreet line.
- Added a centered Alt+Tab switcher showing open application icons. Hold Alt, press Tab repeatedly to select the next app, use Shift+Alt+Tab to cycle backwards, and release Alt to activate the selection. Press Escape to cancel.
- Added keyboard-accessible desktop shortcuts: Ctrl+Alt+D enters or leaves desktop shortcut focus. Use arrow keys to select one of eight shortcuts, Enter to launch, or Escape to cancel. Focus is marked by a small underline rather than a tile.
- Retained working close/minimize/maximize controls, tweened window animations, window snapping and resizing, taskbar previews, Notification Center, Quick Panel, customizable appearance, and all ten applications.

## Keyboard shortcuts

| Keys | Action |
|---|---|
| Alt+Tab | Preview and switch open windows; release Alt to commit |
| Shift+Alt+Tab | Cycle backwards through open windows |
| Escape | Cancel switcher, desktop selection, or panels |
| Ctrl+Alt+D | Toggle desktop shortcut keyboard focus |
| Arrows, Enter | Select and open a focused desktop shortcut |
| Ctrl+1 through Ctrl+7 | Launch pinned apps |
| Alt+F4 | Close active app |
| Alt+D | Show or restore desktop |
| Ctrl+Alt+N | Toggle Notification Center |
| Ctrl+Alt+Left / Right | Snap active window |

## Build and verification

Use Linux with GCC, GNU binutils, Python 3 and 32-bit compilation support.

```sh
make verify
```

The build produces `Cinmix-0.12.iso`. Tests validate ISO and boot structures, window controls through simulated PS/2 mouse packets, existing desktop behavior, compositor updates, and new 0.12 keyboard/icon interactions. Native tests and static ISO checks are **not** a substitute for full BIOS emulator testing. Visual smoothness and runtime performance must be verified in your emulator.

Software volume controls do not yet output audio through a hardware driver. Font licensing is in `FONT-LICENSE.txt`.
