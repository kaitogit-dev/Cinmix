CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy
CFLAGS = -m32 -std=c11 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-ident -O2 -Wall -Wextra -Werror
LDFLAGS = -m elf_i386 -T src/link.ld -nostdlib --build-id=none

all: Cinmix-0.22.iso Cinmix-0.22-disk.img

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

build/kernel.o: src/kernel.c src/objects.c src/ntos.c src/storage.c src/pinta.c src/proc.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/proc.o: src/proc.c src/proc.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build/proc-asm.o: src/proc.S | build
	$(AS) --32 -o $@ $<

build/kernel.elf: build/entry.o build/kernel.o build/font.o build/proc.o build/proc-asm.o src/link.ld
	$(LD) $(LDFLAGS) -o $@ build/entry.o build/kernel.o build/font.o build/proc.o build/proc-asm.o

build/kernel.bin: build/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@test $$(stat -c%s $@) -le 131072 || (echo 'Kernel exceeds CD loader capacity' && exit 1)

Cinmix-0.22.iso: build/cdboot.bin build/kernel.bin README.txt tools/iso.py
	python3 tools/iso.py build/cdboot.bin build/kernel.bin README.txt $@

Cinmix-0.22-disk.img: tools/disk.py
	python3 tools/disk.py $@

verify: Cinmix-0.22.iso
	python3 tools/verify.py $< build/cdboot.bin build/kernel.bin build/kernel.elf
	$(MAKE) smoke

NATIVE_TESTS = build/smoke build/ui11 build/compositor build/controls build/ui12 build/ui13 build/ui131 build/animations build/ui133 build/fullhd build/ui15 build/ui16 build/ui17 build/storage build/ui18 build/objects build/ui19 build/ntos build/ui20 build/proc-test build/pinta-user-test build/ui22

build/%: tests/%.c src/kernel.c src/storage.c src/objects.c src/ntos.c src/pinta.c src/font.c | build
	$(CC) -std=gnu11 -O1 -o $@ $< src/font.c

build/proc-test: tests/proc.c src/proc.c src/proc.h | build
	$(CC) -std=gnu11 -O1 -no-pie -o $@ $<

build/pinta-user-test: tests/pinta_user.c src/proc.c src/proc.h src/pinta.c | build
	$(CC) -std=gnu11 -O1 -no-pie -o $@ $<

smoke: $(NATIVE_TESTS) | build
	./build/smoke
	./build/ui11
	./build/compositor
	./build/controls
	./build/ui12
	./build/ui13
	./build/ui131
	./build/animations
	./build/ui133
	./build/fullhd
	./build/ui15
	./build/ui16
	./build/ui17
	rm -f build/persistence-test.img build/unsigned-test.img build/ui18-test.img
	python3 tools/disk.py build/persistence-test.img
	./build/storage build/persistence-test.img save
	./build/storage build/persistence-test.img reload
	./build/storage build/persistence-test.img recover
	truncate -s 16777216 build/unsigned-test.img
	./build/storage build/unsigned-test.img unsigned
	python3 tools/disk.py build/ui18-test.img
	./build/ui18 build/ui18-test.img
	rm -f build/objects-test.img build/ui19-test.img
	python3 tools/disk.py build/objects-test.img
	./build/objects build/objects-test.img save
	./build/objects build/objects-test.img reload
	./build/objects build/objects-test.img recover
	python3 tools/disk.py build/ui19-test.img
	./build/ui19 build/ui19-test.img
	rm -f build/ntos-test.img build/ui20-test.img
	python3 tools/disk.py build/ntos-test.img
	./build/ntos build/ntos-test.img save
	./build/ntos build/ntos-test.img reload
	./build/ntos build/ntos-test.img recover
	python3 tools/disk.py build/ui20-test.img
	./build/ui20 build/ui20-test.img
	./build/proc-test
	./build/pinta-user-test
	rm -f build/pinta-22.img
	python3 tools/disk.py build/pinta-22.img
	./build/ui22 build/pinta-22.img save
	./build/ui22 build/pinta-22.img reload

bench: | build
	$(CC) -std=gnu11 -O2 -o build/bench tests/bench.c src/font.c
	./build/bench

clean:
	rm -rf build Cinmix-0.22.iso

.PHONY: all verify smoke bench clean
