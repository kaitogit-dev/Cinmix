# Cinmix 0.21 — Protected user mode and scheduling

Cinmix is a freestanding 32-bit x86 desktop OS for legacy BIOS systems, with a 1920×1080 Aero desktop, persistent CinmixFS storage, NT-style file APIs and a new hardware privilege boundary.

## What is implemented in 0.21

- **Real x86 protection levels:** A GDT with ring-0 and ring-3 code/data selectors, a 32-bit task-state segment (TSS) and kernel stack switching on entry to ring 0.
- **Paging and address-space isolation:** Page Size Extensions (PSE) provide two independent 4 MiB ring-3 address spaces at virtual address `0x40000000`. Each maps to different physical memory. Kernel identity mappings are supervisor-only; ring-3 code cannot read the framebuffer, kernel or another task's data through those mappings. The kernel remains flat-mapped at ring 0.
- **Preemptive user-task scheduling:** PIT channel 0 and IRQ0 (interrupt `0x20`) are configured for 100 Hz while a user slice runs. The scheduler saves complete 32-bit general-purpose and segment register contexts, rotates between two real ring-3 test tasks and resumes the ring-0 desktop between bounded execution slices. PIC interrupts are masked when the UI takes control again.
- **Native system-call boundary:** An interrupt gate at `int 0x80` with DPL 3 accepts GetPid, GetTicks and Yield requests. Unknown calls fail with a status code rather than dereferencing kernel pointers.
- **Privilege fault handling:** General protection (`#GP`) and page fault (`#PF`) handlers stop the offending test task instead of treating its error as a normal window exception. Console `faulttest` arms a deliberate privileged-instruction test. Console `userstat` reports task counters, user time slices, preemptions and system calls.
- **Preserved 0.20 features:** Aero and Dark Aero, File Explorer, Settings, Notes, object manager, IRP-based file operations, CinmixFS ATA disk persistence, taskbar and window animations.

## Exact scope and limitations

The two processes are **native ring-3 scheduler demonstration tasks**, not yet GUI applications. The existing desktop and built-in apps still execute inside one ring-0 address space. User-space processes do not yet load ELF/PE executables, create threads dynamically, map files, or use a full NT system-call interface. PSE large pages provide 4 MiB granularity rather than per-page read-only/write protection. This is not a replacement for the Windows NT kernel, nor does it run Win32 applications.

The scheduler's PIT interrupts are active during short user-mode slices; the existing polled desktop regains control between slices. Kernel-mode activity is not yet preemptively scheduled. If the processor lacks PSE support, the desktop still starts without ring-3 scheduling.

## Building

Linux with GCC 32-bit compilation support, GNU binutils and Python 3:

```sh
make verify
make Cinmix-0.21-disk.img
```

The bootable ISO is `Cinmix-0.21.iso`. Attach a writable `Cinmix-0.21-disk.img` as primary IDE master for persistent files, or reuse your existing Cinmix 0.18 through 0.20 disk image. **Do not replace a disk image containing saved data with the blank release image.**

Example QEMU command:

```sh
qemu-system-i386 -m 256 -vga std -cdrom Cinmix-0.21.iso -boot d -drive file=Cinmix-0.21-disk.img,format=raw,if=ide,index=0
```

## Verification status

`make verify` checks ISO9660/El Torito structures and machine-code symbols; native tests exercise page directory permissions, ring-3 interrupt-frame construction, simulated preemption and syscall dispatch, exception isolation, and all existing UI/NT I/O/storage regression suites. The model tests do **not** execute privileged instructions on real hardware. If no BIOS emulator is available, an actual ring-3 context switch and IRQ0 execution remains **unverified** and should be tested in a compatible VM before use with valuable data.
