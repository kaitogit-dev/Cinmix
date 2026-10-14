#include "pe.h"
#include <stddef.h>

static uint16_t pe_u16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8)); }
static uint32_t pe_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void pe_w32(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); p[2]=(uint8_t)(v>>16); p[3]=(uint8_t)(v>>24);
}
static int pe_range(uint32_t offset, uint32_t length, uint32_t total) {
    return offset <= total && length <= total - offset;
}
static void pe_copy(uint8_t *dest, const uint8_t *src, uint32_t length) {
    for (uint32_t i=0;i<length;i++) dest[i]=src[i];
}
static void pe_zero(uint8_t *dest, uint32_t length) {
    for (uint32_t i=0;i<length;i++) dest[i]=0;
}
static int pe_name(const uint8_t *base, uint32_t capacity, uint32_t rva, char *name, uint32_t n) {
    if (rva>=capacity || n<2) return 0;
    for (uint32_t i=0;i<n;i++) {
        if (!pe_range(rva+i,1,capacity)) return 0;
        uint8_t c=base[rva+i];
        if (!c) { name[i]=0; return i>0; }
        if (c<32 || c>126 || i==n-1) return 0;
        name[i]=(char)c;
    }
    return 0;
}
static int pe_equal_fold(const char *a, const char *b) {
    while (*a && *b) {
        char x=*a++,y=*b++;
        if (x>='A'&&x<='Z') x+=32;
        if (y>='A'&&y<='Z') y+=32;
        if (x!=y) return 0;
    }
    return !*a && !*b;
}
const char *cm_pe_error(int status) {
    if (status==CM_PE_OK) return "PE32 application ready";
    if (status==CM_PE_UNSUPPORTED_MACHINE) return "Requires 32-bit x86 PE32";
    if (status==CM_PE_UNSUPPORTED_RUNTIME) return "CLR/.NET, DLL or subsystem unsupported";
    if (status==CM_PE_IMPORT_MISSING) return "Windows DLL/API not yet implemented";
    if (status==CM_PE_NO_SPACE) return "Executable exceeds current address space";
    return "Invalid or unsafe PE executable";
}
int cm_pe_load(const uint8_t *file, uint32_t length, uint8_t *space, uint32_t capacity,
               CmPeImage *image, CmPeResolver resolver) {
    if (!file || !space || !image || !pe_range(0,64,length) ||
        pe_u16(file)!=0x5a4du) return CM_PE_BAD_IMAGE;
    uint32_t offset=pe_u32(file+0x3c);
    if (!pe_range(offset,24,length) || pe_u32(file+offset)!=0x00004550u) return CM_PE_BAD_IMAGE;
    const uint8_t *coff=file+offset+4;
    uint16_t machine=pe_u16(coff);
    if (machine!=0x014cu) return CM_PE_UNSUPPORTED_MACHINE;
    uint16_t nsections=pe_u16(coff+2);
    uint16_t optional_size=pe_u16(coff+16);
    if (!nsections || nsections>16 || optional_size<224 || !pe_range(offset+24,optional_size,length) ||
        (pe_u16(coff+18)&0x2000u)) return CM_PE_BAD_IMAGE;
    const uint8_t *opt=coff+20;
    if (pe_u16(opt)!=0x10bu) return CM_PE_UNSUPPORTED_MACHINE;
    uint32_t entry=pe_u32(opt+16), imagebase=pe_u32(opt+28), image_size=pe_u32(opt+56);
    uint32_t header_size=pe_u32(opt+60), section_alignment=pe_u32(opt+32);
    uint32_t file_alignment=pe_u32(opt+36), directories=pe_u32(opt+92);
    uint16_t subsystem=pe_u16(opt+68);
    if (subsystem!=3) return CM_PE_UNSUPPORTED_RUNTIME;
    if (imagebase!=CM_PE_BASE || section_alignment<4096 || !file_alignment ||
        image_size<0x2000 || image_size>CM_PE_LIMIT || image_size>capacity)
        return image_size>capacity || image_size>CM_PE_LIMIT ? CM_PE_NO_SPACE : CM_PE_UNSUPPORTED_RUNTIME;
    if (!pe_range(0,header_size,length) || header_size>image_size ||
        !pe_range(offset+24+optional_size,nsections*40u,length) || directories<16)
        return CM_PE_BAD_IMAGE;
    if (pe_u32(opt+96+14*8)!=0 || pe_u32(opt+96+14*8+4)!=0) return CM_PE_UNSUPPORTED_RUNTIME;
    uint32_t import_rva=pe_u32(opt+96+8), import_size=pe_u32(opt+96+12);
    if ((import_rva && !import_size) || (!import_rva && import_size)) return CM_PE_BAD_IMAGE;
    pe_zero(space,image_size);
    pe_copy(space,file,header_size);
    const uint8_t *sections=opt+optional_size;
    int has_entry=0;
    for (uint32_t i=0;i<nsections;i++) {
        const uint8_t *s=sections+i*40;
        uint32_t virtual_size=pe_u32(s+8), rva=pe_u32(s+12);
        uint32_t raw_length=pe_u32(s+16), raw_offset=pe_u32(s+20);
        uint32_t mapped=virtual_size>raw_length?virtual_size:raw_length;
        if ((rva&0xfffu) || !pe_range(rva,mapped,image_size) ||
            (raw_length && !pe_range(raw_offset,raw_length,length))) return CM_PE_BAD_IMAGE;
        if (raw_length) pe_copy(space+rva,file+raw_offset,raw_length);
        if (entry>=rva && entry<rva+mapped && (pe_u32(s+36)&0x20000000u)) has_entry=1;
    }
    if (!has_entry || entry==0) return CM_PE_BAD_IMAGE;
    uint32_t count=0;
    if (import_rva) {
        if (!resolver || !pe_range(import_rva,import_size,image_size) || import_size<40) return CM_PE_BAD_IMAGE;
        int terminated=0;
        for (uint32_t d=0;d<16;d++) {
            uint32_t desc=import_rva+d*20u;
            if (!pe_range(desc,20,image_size) || !pe_range(desc,20,import_rva+import_size)) return CM_PE_BAD_IMAGE;
            uint32_t lookup=pe_u32(space+desc), name=pe_u32(space+desc+12), target=pe_u32(space+desc+16);
            if (!lookup && !name && !target) { terminated=1;break; }
            char dll[48];
            if (!pe_name(space,image_size,name,dll,sizeof(dll)) || !pe_equal_fold(dll,"KERNEL32.DLL")) return CM_PE_IMPORT_MISSING;
            if (!lookup) lookup=target;
            for (uint32_t k=0;k<64;k++) {
                if (!pe_range(lookup+k*4,4,image_size) || !pe_range(target+k*4,4,image_size)) return CM_PE_BAD_IMAGE;
                uint32_t item=pe_u32(space+lookup+k*4);
                if (!item) break;
                if (item&0x80000000u || !pe_range(item,3,image_size)) return CM_PE_IMPORT_MISSING;
                char symbol[64];
                if (!pe_name(space,image_size,item+2,symbol,sizeof(symbol))) return CM_PE_BAD_IMAGE;
                uint32_t address=resolver(dll,symbol);
                if (!address) return CM_PE_IMPORT_MISSING;
                pe_w32(space+target+k*4,address);
                count++;
                if (k==63) return CM_PE_BAD_IMAGE;
            }
        }
        if (!terminated) return CM_PE_BAD_IMAGE;
    }
    image->entry=CM_PE_BASE+entry;
    image->image_base=CM_PE_BASE;
    image->image_size=image_size;
    image->import_count=count;
    image->subsystem=subsystem;
    return CM_PE_OK;
}
