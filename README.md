# Cinmix 0.23 — PE32 Loader and Minimal Win32 Console Compatibility

Cinmix is a freestanding 32-bit x86 hobby OS that boots from a legacy BIOS El Torito CD, with a 1920×1080 Aero/Dark Aero desktop, NT-like kernel APIs and a persistent CinmixFS disk.

## Real changes in 0.23

- **PE32 executable loading:** DOS MZ and PE/COFF header verification, Intel 386 machine/PE32 optional header validation, bounded section mapping, section entry checks, and import-address-table patching. The loader maps images at `0x40000000` in a ring-3 task, with an isolated 4 MiB paging mapping. It rejects malformed images, DLLs, unsupported architectures, missing imports and CLR/.NET executables rather than pretending they work.
- **Tiny KERNEL32 bridge:** import resolution for `GetCurrentProcessId`, `GetTickCount`, and `ExitProcess`. In the protected user process, these functions use system-call trampolines and `int 0x80` to enter the kernel. `ExitProcess` terminates the user task and reports its exit code.
- **Disk-backed Windows executable:** the bundled `hello.exe` is a small, valid PE32 console executable whose entry point imports the three KERNEL32 routines, writes two results to private process memory and exits with code `42`. It is *not* a normal full-featured Windows executable. The downloadable disk image includes it preinstalled. Other small supported PE32 executables may be added with `tools/install_pe.py`, subject to the strict limits below.
- **Launch from the Console:** type `runpe hello.exe`, then `peinfo` to see the program exit code and launch counts. `compat` reports exactly what is and isn't available. File Explorer lists `hello.exe` under D:, identifies it as a Windows executable and launches a selected executable when you press Enter.
- **Boot-loader capacity:** the El Torito loader now reads 80 two-kibibyte sectors, and the ISO verifier checks the corresponding kernel padding. All previous desktop, painter, protected-worker, and persistent-storage features remain.

## .NET 10, GTK4/libadwaita, and arbitrary Windows apps

**Not installed and not supported.** .NET 10 requires a substantial managed-code runtime (CoreCLR, the class libraries, a JIT/interpreter, native platform interfaces, and support for its target architecture). GTK4/libadwaita requires a much larger graphics/windowing, fonts, internationalization and platform-services stack. These libraries do not themselves provide Win32 compatibility. Most Windows apps depend on extensive Win32 DLLs (USER32, GDI32, ADVAPI32, KERNEL32, COM, DirectX, etc.), filesystem behavior, and often Windows drivers. The existing PE32 loader is a narrow first step rather than a Windows emulator.

**Current executable constraints:** 32-bit x86 **PE32 console** images only; one ring-3 executable at a time (alongside the painting worker); **4 KiB or less per executable on CinmixFS v1**; fixed image base `0x40000000`; no relocation support; only the three imported KERNEL32 functions above; no CRT or heap API, no GUI, no DLL loading, no Win32 exceptions, no .NET CLR, no GTK, and no x64 support. An unsupported EXE is rejected with an error, not launched.

Pinta remains a **native Cinmix painter inspired by** Pinta's basic workflow; this is not the upstream .NET 10/GTK4 program. Attribution stays in `third_party/`.

## Running the demo

Attach **both** the ISO and writable disk image in QEMU or another BIOS x86 emulator. Example:

```sh
qemu-system-i386 -m 256 -vga std \
  -cdrom Cinmix-0.23.iso -boot d \
  -drive file=Cinmix-0.23-disk.img,format=raw,if=ide,index=0
```

Open Console, type `runpe hello.exe` and press Enter. Once it finishes, type `peinfo`. The expected exit code is `42`. The executable's imports use real loaded PE IAT entries and the ring-3 syscall interface. `hello.exe` can also be selected in Explorer and launched with Enter.

Existing CinmixFS volumes from 0.18 onward remain **format compatible**. Use your previous disk image if it contains data rather than overwriting it with this demo disk. To install `hello.exe` onto an existing signed disk image after backing it up:

```sh
python3 tools/pe_sample.py hello.exe
python3 tools/install_pe.py your-disk.img hello.exe
```

## Build and verification

```sh
make verify
make Cinmix-0.23-disk.img
```

The tests cover malformed images, unsupported CLR/GUI/x64 images, real PE import resolution, process exit/relaunch, persistent storage installation, ISO structure and all previous desktop/native storage regressions. **The protected execution path has not been boot-tested in a full BIOS emulator in this environment.** Keep backups of your writable disk images and test on a disposable copy first.
