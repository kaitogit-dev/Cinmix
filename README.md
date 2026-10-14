# Cinmix 0.20 — NT-style kernel I/O and registry

Cinmix is a freestanding 32-bit x86 operating system with the Aero and Dark Aero desktops, a preferred 1920×1080 display, BIOS El Torito boot support, and an optional persistent CinmixFS IDE hard disk.

## What 0.20 actually implements

- **NT native file API subset**: `NtCreateFile`, `NtReadFile`, `NtWriteFile`, `NtFlushBuffersFile`, `NtQueryInformationFile`, and `NtClose`, using `NTSTATUS`, `IO_STATUS_BLOCK`, `UNICODE_STRING`, and `OBJECT_ATTRIBUTES` structures. `FILE_OPEN`, `FILE_CREATE`, `FILE_OPEN_IF`, and `FILE_OVERWRITE_IF` are supported for ordinary 8-bit Cinmix filenames within the existing 4 KB-per-file limit.
- **I/O Manager and IRPs**: a `DRIVER_OBJECT` registers an IDE-backed CinmixFS `DEVICE_OBJECT`. File operations traverse `IoCallDriver`, IRP major-function dispatch, and the existing generation-checked Cinmix file backend. Unsupported requests return NT-style error statuses instead of silently succeeding.
- **NT object namespace subset**: `\\Device\\CinmixDisk` with a DOS symbolic link `\\??\\D:`. Path resolution rejects unknown devices, traversal, and illegal filenames. New generation-checked NT file and registry handles prevent use-after-close.
- **Registry API subset**: `NtCreateKey`, `NtOpenKey`, `NtSetValueKey`, and `NtQueryValueKey` for `\\Registry\\Machine\\Software\\Cinmix` and DWORD values. The Theme value is saved through the NT file APIs into a validated `system.reg` file on the persistent disk, and restored at boot. The registry file has a version and CRC and uses the existing copy-on-write CinmixFS mechanism.
- **Real integration**: Notes and Explorer now create, read, write, flush, and close their files through the NT I/O layer, rather than reaching straight into CinmixFS. Console `handles` includes the NT handle count alongside the existing window object handles.
- **Compatibility**: Existing CinmixFS 0.18/0.19 disk images retain their original on-disk layout and saved documents. The new registry file uses one of the existing 16 file slots. You can continue to use your current disk image; do not overwrite it with the blank example disk.

## Windows compatibility status

This is a **limited implementation of real NT interface names, layouts, statuses, object paths, I/O request packets, and driver dispatch flow**. It is *not* a Windows NT kernel or a bit-for-bit copy of proprietary Windows code. It cannot load Windows drivers or run Windows executables. It has no Windows Registry hive (`regf`) compatibility, security access tokens, APC completion, kernel/user privilege separation, isolated processes, virtual memory, PE loader, preemptive scheduler, NTFS, or Win32 subsystem. The current kernel/apps still share one address space. Those require separate, much larger changes.

This release intentionally implements and tests its supported NT API paths rather than implying the unsupported components exist. The `system.reg` file is Cinmix's own private registry backing store, not a Windows hive.

## Booting with a writable disk

Use the ISO and **an existing writable disk image** containing your files, or the newly formatted blank disk image if you have no saved documents yet:

```bash
qemu-system-i386 -m 256 -vga std -cdrom Cinmix-0.20.iso -boot d \
  -drive file=Cinmix-0.20-disk.img,format=raw,if=ide,index=0
```

The hard disk must be presented as the primary IDE master and carry the CinmixFS signature. Cinmix will not automatically format an unknown disk. Without a supported disk, Cinmix boots in temporary RAM-only mode.

## Building and testing

Requires GNU assembler, GCC (`-m32` support), GNU linker, objcopy, and Python 3.

```bash
make verify
make all
```

`make verify` checks the ISO9660/El Torito structure, the expanded CD sector loader, existing UI/animation and 1080p tests, save/restore of the original CinmixFS format, NT namespace and status validation, IRP routing, handle generation, registry persistence across distinct native processes, failed metadata recovery, and 1080p Notes/Explorer integration. A clean-source rebuild should match the ISO byte for byte.

**Native tests are not a full BIOS emulator run.** Actual VBE and IDE operation in QEMU or v86 remains unverified here. Back up writable disk images before experimenting.
