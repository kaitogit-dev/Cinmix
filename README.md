# Cinmix 0.15 — Dark Aero

Cinmix is a freestanding 32-bit x86 BIOS desktop operating system. It prefers a 1920×1080×32 VBE or Bochs-compatible graphics mode, with lower-resolution fallbacks. The interface combines Aero-style glass, rounded antialiased edges, a centered icon-only taskbar, and animated application windows.

## What is new in 0.15

**Dark Aero** is a complete fourth color scheme alongside Aero, Silver and Midnight. It introduces a dark-blue translucent wallpaper, navy title bars and window content, glass panels, dark text fields and buttons, and high-contrast light text throughout Files, Notes, System, Calculator, Appearance, Start, Run, Quick Panel, Notification Center, Alt+Tab and taskbar tooltips. Dark Aero uses the existing configurable accent colors and glass strength. Switching back restores the last selected light theme rather than resetting other appearance preferences.

**Dark Aero controls:**

- In Appearance, click the **Dark Aero** button at the top right of Color Scheme.
- In the Quick Panel (click the clock), click **Dark Aero: On/Off**.
- Right-click an empty desktop area and choose **Toggle Dark Aero**.
- Press **Ctrl+Alt+M** at any time to toggle Dark Aero.
- Type **dark** or **light** in Run (`Ctrl+R`) or Console.

**Quiet mode / Do Not Disturb:** The Quick Panel now includes **Quiet: On/Off**. New notifications are still saved in the Notification Center but do not create unread badges while quiet mode is enabled. Existing notifications can still be read and dismissed normally. Quiet mode can also be toggled with `quiet` in Run or Console.

**New window shortcuts:** **Ctrl+Alt+Up** maximizes or restores the active window, and **Ctrl+Alt+Down** minimizes it with the existing closing tween. Existing left/right snapping shortcuts remain unchanged.

**Render optimization:** Wallpaper caching now distinguishes light and dark appearances without regenerating a full-HD wallpaper when switching between light themes. The renderer preserves partially redrawn window and panel regions and the cursor compositing fixes from previous versions.

## Existing desktop features

Ten built-in apps, searchable Start menu, desktop right-click menu, reversible window/panel tweens, dragging, resizing, snapping, minimize and restore, animated Show Desktop, running and hovered taskbar highlights, delayed taskbar tooltips, notification management, quick volume UI, wallpapers, glass strength and accent selection, Run dialog, visual Alt+Tab, and keyboard desktop icon selection.

## Build and verification

On a Linux system with GCC, GNU binutils, Python 3 and 32-bit cross-compilation support:

```sh
make verify
```

This creates `Cinmix-0.15.iso`. Tests validate ISO9660 and El Torito structures, BIOS video mode selection, panel and window controls, 1920×1080 framebuffer composition, both color modes, theme input controls, quiet notifications and the new keyboard shortcuts. The tests run against the native software compositor and simulated input; they **do not establish actual boot or frame-rate smoothness in a BIOS emulator**. The emulator must provide a suitable 1080p framebuffer mode or Cinmix falls back to a supported resolution. At least 128 MB of RAM is recommended. Preferences remain in RAM and reset when Cinmix reboots.

Software volume controls are interface state only; an audio device driver has not been implemented. Font licensing information is in `FONT-LICENSE.txt`.
