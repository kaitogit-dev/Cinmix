# Cinmix 0.22 — Protected Painter Worker and Native Pinta-Style App

Cinmix is a freestanding 32-bit x86 hobby operating system for legacy BIOS El Torito boot, targeting a 1920×1080 desktop with Aero / Dark Aero, Notes, Explorer, Settings, and persistent CinmixFS storage.

## New in 0.22

### Protected painting worker (NT architecture milestone)

The process manager's first ring-3 task now accepts bounded raster **stamp requests** through a per-process mailbox at virtual address `0x40002100`. The task paints into its isolated user-space buffer at `0x40010000` and signals completion; the ring-0 desktop validates and collects the result before composing the next frame. The mailbox validates coordinates, palette indices, and brush radius. A faulted or stopped worker is not used. Timer-based preemption, ring-3 pages, TSS stack transitions, and `int 0x80` from 0.21 remain in place.

**Scope:** This is an actual isolated user-mode painting operation, not an entire GUI process. The desktop, compositor, menus, shape geometry, mouse input and persistent file I/O are still handled in kernel mode. It does not implement a full NT process loader, Win32 API, or security model.

### Painting application replaces Sketch

The old Sketch application has been replaced by **a native Cinmix painter modeled on Pinta's basic workflow**, with a Pinta launcher label. Features include:

- Pencil, brush, eraser, line, rectangle and bucket-fill tools;
- Adjustable brush radius, expanded color palette and a larger painting surface;
- Undo, redo, and clear;
- `Ctrl+S` and a Save button, storing the canvas as segmented `painting0.cmx` through `painting6.cmx` in the existing CinmixFS volume, accessed through Cinmix's NT-style file APIs;
- Restoring a saved painting when the OS boots again with the same writable disk image.

**Important compatibility distinction:** [Pinta](https://github.com/PintaProject/Pinta) itself is a .NET 10 / GTK4 / libadwaita application. That original binary and upstream C# interface **cannot run in Cinmix** today; this native C implementation is a **limited reimplementation, not the original Pinta executable or a full source port**. It does not include Pinta's complete tools, effects, layers, plugin infrastructure, GTK UI, or original artwork. The original project's license information and attribution are in `third_party/PINTA-LICENSE.txt` and `third_party/PINTA-SOURCE.txt`. No code or artwork from upstream Pinta is compiled into the kernel.

## Persistence and disk compatibility

The existing 16 MiB CinmixFS image format is unchanged. **Keep using your existing 0.18–0.21 disk image if it contains saved files.** Do not overwrite it with a blank image. This release's painting consumes **seven of the filesystem's sixteen 4 KiB slots**, leaving fewer available file slots for Notes and Explorer documents. Writes require a writable, signed CinmixFS disk; without one, paintings can be edited during the session but will not persist through reboot.

## Build and verify

On Linux with a C compiler supporting `-m32`, GNU binutils and Python 3:

```sh
make verify
make Cinmix-0.22-disk.img
```

The bootable ISO is `Cinmix-0.22.iso`. To test in QEMU with its persistent disk image:

```sh
qemu-system-i386 -m 256 -vga std \
  -cdrom Cinmix-0.22.iso -boot d \
  -drive file=Cinmix-0.22-disk.img,format=raw,if=ide,index=0
```

The native regression suite tests the 1920×1080 compositor and desktop apps, protected page mapping and mailbox validation, brush results, painting tools, undo/redo, NT file calls, and save/reload through a **separate process**. ISO verifier checks BIOS boot structure and kernel instructions. **A full BIOS emulator boot and live hardware ring-3 paint execution are not verified in this environment.** Keep backups of important disk images.
