import pathlib
import struct
import sys

BASE = 0x40000000


def generate(path):
    data = bytearray(2048)
    data[0:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    off = 0x80
    data[off:off + 4] = b'PE\0\0'
    struct.pack_into('<HHIIIHH', data, off + 4, 0x14c, 3, 0, 0, 0, 224, 0x0102)
    o = off + 24
    struct.pack_into('<H', data, o, 0x10b)
    struct.pack_into('<IIIIII', data, o + 4, 0x200, 0x400, 0, 0x1000, 0x1000, 0x2000)
    struct.pack_into('<IIII', data, o + 28, BASE, 0x1000, 0x200, 4)
    struct.pack_into('<HHHHHH', data, o + 40, 0, 0, 0, 0, 4, 0)
    struct.pack_into('<I', data, o + 56, 0x4000)
    struct.pack_into('<I', data, o + 60, 0x200)
    struct.pack_into('<HH', data, o + 68, 3, 0)
    struct.pack_into('<IIIIII', data, o + 72, 0x4000, 0x1000, 0x4000, 0x1000, 0, 16)
    struct.pack_into('<II', data, o + 104, 0x2000, 40)
    sections = off + 24 + 224
    for n, (name, va, raw, flags) in enumerate([
        (b'.text', 0x1000, 0x200, 0x60000020),
        (b'.idata', 0x2000, 0x400, 0xc0000040),
        (b'.data', 0x3000, 0x600, 0xc0000040),
    ]):
        at = sections + n * 40
        data[at:at + 8] = name.ljust(8, b'\0')
        struct.pack_into('<IIIIII', data, at + 8, 0x200, va, 0x200, raw, 0, 0)
        struct.pack_into('<I', data, at + 36, flags)
    def movabs(address):
        return b'\xa3' + struct.pack('<I', address)
    code = (b'\xff\x15' + struct.pack('<I', BASE + 0x2070) + movabs(BASE + 0x3000) +
            b'\xff\x15' + struct.pack('<I', BASE + 0x2074) + movabs(BASE + 0x3004) +
            b'\x6a\x2a' + b'\xff\x15' + struct.pack('<I', BASE + 0x2078) + b'\xf4')
    data[0x200:0x200+len(code)] = code
    struct.pack_into('<IIIII', data, 0x400, 0x2050, 0, 0, 0x20a0, 0x2070)
    for k, rva in enumerate((0x20c0, 0x20e0, 0x2100)):
        struct.pack_into('<I', data, 0x450 + 4*k, rva)
        struct.pack_into('<I', data, 0x470 + 4*k, rva)
    data[0x4a0:0x4a0+13] = b'KERNEL32.DLL\0'
    for rva, name in ((0x20c0, b'GetCurrentProcessId'), (0x20e0, b'GetTickCount'), (0x2100, b'ExitProcess')):
        offset = 0x400 + rva - 0x2000
        data[offset:offset+2] = b'\0\0'
        data[offset+2:offset+2+len(name)+1] = name+b'\0'
    pathlib.Path(path).write_bytes(data)
    print(f'Generated minimal, valid PE32 Windows console executable: {path} ({len(data)} bytes)')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('Usage: python3 tools/pe_sample.py <hello.exe>')
    generate(sys.argv[1])
