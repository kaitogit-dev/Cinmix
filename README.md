# Cinmix 0.17 — Windows-style Settings and Explorer controls

Cinmix is a freestanding, 32-bit i386 desktop OS booting from a legacy BIOS El Torito CD image, targeting **1920×1080** with a Windows 7 Aero and Windows 11-inspired appearance.

## New in 0.17

- **Windows 11–inspired Settings app:** The old Appearance window is now Settings. It has a sidebar with **System**, **Personalization**, **Apps**, **Notifications**, and **About** pages. Choose light or Dark Aero, select themes, wallpapers, glass strength and accent colors; control software volume and Quiet mode; clear notification history; view display and storage information; or open installed apps directly from Settings.
- **Settings search:** Click the search field, enter a setting category such as `theme`, `display`, `volume`, `apps`, `quiet`, `wallpaper`, or `about`, and press Enter. `Ctrl+F` focuses search while Settings is active.
- **File Explorer controls work:** **New** creates one of three temporary RAM documents. **Sort** reverses the file order; **View** switches between details and a taller list layout; **...** shows or hides the details pane. The left navigation filters files by virtual category. The breadcrumb returns to RAM. Search supports keyboard input, backspace, and Escape; `Ctrl+F` focuses it. Click a row to select, use Up/Down to move selection, or press Enter to open a supported item in Notes, Settings or Calendar.
- **Responsive Settings layout:** Compact spacing keeps Personalization controls reachable in 1280×720 windows as well as the preferred 1920×1080 mode.
- **Preserved:** Dark Aero, mouse cursor, desktop icon hover highlights, taskbar previews and running highlights, animated windows and panels, Notification Center, Run, sketching and other built-in apps.

## Current storage limitations

Explorer is an **in-memory virtual RAM browser**, not yet a driver-backed NTFS/FAT filesystem. Its preloaded entries are demonstrations and new RAM documents disappear after reboot. The Notes app remains a volatile text editor, and audio volume is a software UI value without an audio device driver. Display resolution is selected by compatible firmware at boot rather than dynamically in Settings.

## Build and test

On Linux with GCC (including `-m32` support), GNU binutils and Python 3:

```sh
make verify
```

This builds `Cinmix-0.17.iso`, checks ISO9660/El Torito structure and the 56-sector kernel bootloader, and executes **13 native test suites**, including a real-mouse-packet test of Explorer and Settings at 1920×1080. The larger bootloader image supports up to 112 KiB of kernel code, instead of the previous 96 KiB limit. Native tests and ISO structure checks **do not prove successful boot or smooth performance in a BIOS emulator**. Full BIOS emulator execution still needs to be confirmed in your environment.

All Cinmix preferences and RAM file entries reset on reboot. Font licensing information is in `FONT-LICENSE.txt`.
