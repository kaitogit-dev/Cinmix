# Cinmix 0.13.2

Cinmix is a freestanding 32-bit x86 desktop operating system designed for a legacy BIOS El Torito CD image at 1280 x 720. Its desktop combines Aero-style glass, a centered icon-only taskbar, anti-aliased edges, animated windows, and lightweight built-in apps.

## Fixed in 0.13.2

- Reversible opening and closing tweens for Start, desktop context menu, Quick Panel, Notification Center, Run, Alt+Tab, and taskbar hover tooltips.
- Closing a popup partway through its opening animation now reverses smoothly instead of snapping away.
- Fixed stale popup pixels remaining on the screen after the last closing frame.
- Show Desktop now uses the regular minimize animation, and restoring the desktop reverses active minimizes or animates windows back.
- These transitions use the existing 60 Hz-paced damage-region compositor rather than forcing full-screen redraws.
- Added `tests/animations.c` with pixel-exact framebuffer checks, click and keyboard close paths, mid-tween reversals, window minimize and restore checks.

## Existing appearance refinements (0.13.1)

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

The build produces `Cinmix-0.13.2.iso`. Tests validate ISO and boot structures, window controls through simulated PS/2 mouse packets, compositor updates, icon and keyboard behavior from 0.12, and new 0.13.2 interactions including delayed taskbar labels, the Run dialog, and silent app launches. Native tests and static ISO checks are **not** a substitute for full BIOS emulator testing. Visual smoothness and runtime performance must be verified in your emulator.

Software volume controls do not yet output audio through a hardware driver. Font licensing is in `FONT-LICENSE.txt`.
