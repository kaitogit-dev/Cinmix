# Cinmix 0.19 — Windows-inspired kernel internals

Cinmix is a freestanding 32-bit i386 desktop operating system with Aero and Dark Aero themes, a 1920×1080 preferred display and BIOS El Torito CD boot support. Version 0.19 preserves all 0.18 desktop features and adds **Windows-inspired infrastructure underneath the interface**.

## What's new inside the OS

- **Task registry:** each of the ten built-in applications gets a stable session-local task ID (PID) and Running, Minimized, or Stopped state. A task is an application identity within the existing single-threaded kernel, **not a hardware-protected process**.
- **Typed kernel handles:** windows and open files use generation-checked opaque 32-bit handles. A handle that is closed and whose slot has been reused is rejected rather than accidentally addressing the new object. The object table supports up to 32 live objects.
- **Window message queue:** create, activate, close, minimize, paint, mouse, key and character messages are posted to a bounded queue with validated destination window handles. Duplicate paint messages are coalesced, and messages for destroyed windows are discarded. The main event loop pumps the queue with a fixed per-iteration budget. Notes and Console editing use the message dispatcher.
- **File-handle API:** open, read, write, seek, flush and close operations on the `D:` drive sit between applications and the CinmixFS storage driver. Notes and Explorer create and save documents through these APIs rather than calling raw filesystem operations. Buffered writes commit through CinmixFS's existing copy-on-write directory and data-bank scheme.
- **Diagnostics:** use `tasklist` in Console to see app PIDs and states, and `handles` to see allocated objects. The System app displays task and handle counts and whether a persistent disk is mounted; Windows shows a PID next to each running window.

This is **inspired by Windows architecture**, but it is **not Windows NT or a Win32 implementation**. It cannot run Windows executables. The kernel, apps and drivers still share a single address space and run cooperatively without per-process memory protection, preemptive scheduling, DLL loading, a registry, virtual memory or Windows drivers. Those would require much larger architectural changes.

## Disk compatibility

**Keep your existing Cinmix 0.18 disk image** if it has saved data. Cinmix 0.19 uses the **same unmodified CinmixFS on-disk format**, including its signed superblock, fixed 16 file slots, 4 KiB per-file limit, and dual directory copies. The optional new `Cinmix-0.19-disk.img` is a blank 16 MiB image; do not overwrite your existing disk with it. Files created through the new handle API are persisted through the same ATA PIO driver.

Use both boot media and one persistent IDE disk image:

```bash
qemu-system-i386 -m 256 -vga std -cdrom Cinmix-0.19.iso -boot d \
  -drive file=Cinmix-0.19-disk.img,format=raw,if=ide,index=0
```

The ISO alone still boots with a temporary RAM-only session. Cinmix never automatically formats an unknown attached drive. Make backups of writable disk images before testing an experimental OS.

## Build and verification

On Linux with GCC including `-m32`, GNU binutils and Python 3:

```bash
make verify
make all
```

Tests include the existing 720p and 1080p UI regressions, ISO boot-structure checks, disk persistence across separate native processes, and new tests of message ordering, paint coalescing, stale window/file handles, file seek/read/write/close, failed metadata commits, and Notes/Explorer saves through the file API. The clean-source rebuild should reproduce the ISO byte-for-byte.

**A full BIOS-emulator boot and real IDE hardware interaction have not been verified in this environment.** Native mocks exercise the code and on-disk format, not QEMU/v86 real execution. The current file system has no directories or FAT/NTFS support, and the volume controls still need an audio driver.
