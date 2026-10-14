# Cinmix 0.18 — Persistent storage

Cinmix is a freestanding 32-bit x86 operating system with a Windows-inspired Aero desktop at 1920×1080. **Version 0.18 adds persistent storage**, while preserving the Windows-style Settings, File Explorer, Dark Aero, window animations and all existing applications.

## What works

- **ATA PIO** 28-bit LBA sector I/O on the **primary IDE master** using ports `0x1F0–0x1F7` and `0x3F6`, with hardware identification and bounded status waits.
- **CinmixFS**: a compact filesystem with a signed superblock, CRC32 checksums, alternating directory copies and copy-on-write data banks. A write that fails before committing directory metadata does not replace the previously saved version.
- **Notes:** `Ctrl+S` saves the current document to the attached disk; closing or minimizing Notes also saves it. Opening a saved document in Explorer loads its contents from the disk. The default `notes.txt` persists too.
- **File Explorer:** New Document creates a real entry on the disk; saved documents appear again when the same disk image is attached at the next boot. The app keeps existing navigation, sorting, searching and details controls.
- **Settings > System > Storage** shows whether the writable disk is detected. The Console `disk` command also reports it.
- **Safety:** Cinmix will not format any disk automatically. It enables writing only if the expected CinmixFS signature, version, geometry and directory checksums are valid. Without a valid disk it stays in temporary RAM-only mode.

## Running Cinmix with a writable disk

Use **two files together**: the bootable `Cinmix-0.18.iso` and the separate `Cinmix-0.18-disk.img` (16 MiB). Use the **same disk image for subsequent boots** to preserve data; **do not download a fresh empty disk image over your saved one**. Back up your image regularly.

For example, in QEMU:

```sh
qemu-system-i386 -m 256 -vga std -cdrom Cinmix-0.18.iso -boot d -drive file=Cinmix-0.18-disk.img,format=raw,if=ide,index=0
```

The CD image boots the OS; the disk image is a writable **primary IDE master hard disk**. The loader does not boot from the disk. To create a new blank CinmixFS image yourself, run `python3 tools/disk.py new-disk.img`. The command refuses to overwrite an existing file. Using the ISO alone still works, but new documents and notes are temporary.

### Filesystem limitations

CinmixFS is **not FAT/NTFS/ext** and cannot be mounted directly by the host operating system. Version 0.18 supports 16 fixed document slots, up to 4096 bytes per file (Notes edits up to 2047 characters). Explorer exposes its three user-created document slots plus built-in demonstration entries; disk storage is not yet a general-purpose filesystem with folders, deletion, and file extension associations. Running programs and the framebuffer still require RAM; the disk removes the loss of saved documents between boots, not the use of RAM itself. The implementation does not handle real hardware DMA, SATA/AHCI or USB storage.

## Build and tests

On Linux with GCC, GNU binutils, Python 3 and `-m32` compilation support:

```sh
make verify
make all
```

`make verify` runs ISO9660 and El Torito structure checks, native UI regressions, persistence tests that save and reopen the disk **across separate native processes**, a simulated failed metadata commit, and end-to-end Explorer/Notes save and remount interactions. `make all` creates both ISO and disk image. The disk image is not regenerated if it already exists. **A full BIOS-emulator boot has not been tested in this environment**; native mocked sector I/O validates the filesystem logic but cannot prove ATA hardware compatibility in every emulator. Never use an existing personal disk with experimental OS software.
