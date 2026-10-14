# Cinmix 0.13.1

Cinmix is a freestanding 32-bit x86 desktop operating system designed for a legacy BIOS El Torito CD image at 1280 x 720. Its desktop combines Aero-style glass, a centered icon-only taskbar, anti-aliased edges, animated windows, and lightweight built-in apps.

## New in 0.13.1

- Enlarged all eight desktop shortcut icons while reducing both row and column spacing.
- Replaced the dark taskbar label popup with a compact light Aero-style tooltip anchored to its icon; running app previews have a consistent lighter header and border.

## Included from 0.13

- Replaced the old hardcoded cursor with a new white SVG-inspired arrow cursor, drawn with anti-aliased edges and a subtle shadow.
- Fixed taskbar hover labels so they wait about one second before appearing, instead of showing immediately.
- Removed generic app-open notifications, so opening Files or other apps no longer spams Notification Center.
- Added a Run dialog. Open it with `Ctrl+R` or from Quick Panel, type commands like `files`, `notes`, `console`, `appearance`, `calendar`, `desktop`, `reboot`, or `shutdown`, then press Enter.
- Retained clean tile-free desktop and taskbar icons, active app underlines, Alt+Tab visual switching, keyboard desktop navigation, resize and snap behavior, Notification Center, Quick Panel, and all ten apps.

## Keyboard shortcuts

| Keys | Action |
|---|---|
| Alt+Tab | Preview and switch open windows; release Alt to commit |
| Shift+Alt+Tab | Cycle backwards through open windows |
| Escape | Cancel switcher, desktop selection, Run dialog, or panels |
| Ctrl+Alt+D | Toggle desktop shortcut keyboard focus |
| Arrows, Enter | Select and open a focused desktop shortcut |
| Ctrl+R | Open or close the Run dialog |
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

The build produces `Cinmix-0.13.1.iso`. Tests validate ISO and boot structures, window controls through simulated PS/2 mouse packets, compositor updates, icon and keyboard behavior from 0.12, and new 0.13.1 interactions including delayed taskbar labels, the Run dialog, and silent app launches. Native tests and static ISO checks are **not** a substitute for full BIOS emulator testing. Visual smoothness and runtime performance must be verified in your emulator.

Software volume controls do not yet output audio through a hardware driver. Font licensing is in `FONT-LICENSE.txt`.
