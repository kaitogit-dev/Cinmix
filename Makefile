CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy
CFLAGS = -m32 -std=c11 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-ident -O2 -Wall -Wextra -Werror
LDFLAGS = -m elf_i386 -T src/link.ld -nostdlib --build-id=none

all: Cinmix-0.10.iso

build:
	mkdir -p build

build/cdboot.o: src/cdboot.S | build
	$(AS) --32 -o $@ $<

build/cdboot.bin: build/cdboot.o
	$(LD) -m elf_i386 -Ttext 0x7c00 --oformat binary -o $@ $<

build/entry.o: src/entry.S | build
	$(AS) --32 -o $@ $<

build/font.o: src/font.c | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/kernel.o: src/kernel.c | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/kernel.elf: build/entry.o build/kernel.o build/font.o src/link.ld
	$(LD) $(LDFLAGS) -o $@ build/entry.o build/kernel.o build/font.o

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@test $$(stat -c%s $@) -le 98304 || (echo 'Kernel exceeds CD loader capacity' && exit 1)

Cinmix-0.10.iso: build/cdboot.bin build/kernel.bin README.txt tools/iso.py
	python3 tools/iso.py build/cdboot.bin build/kernel.bin README.txt $@

verify: Cinmix-0.10.iso
	python3 tools/verify.py $< build/cdboot.bin build/kernel.bin build/kernel.elf
	$(MAKE) smoke

smoke:
	$(CC) -std=gnu11 -O2 -o build/smoke tests/smoke.c src/font.c
	./build/smoke

clean:
	rm -rf build Cinmix-0.10.iso

.PHONY: all verify smoke clean
