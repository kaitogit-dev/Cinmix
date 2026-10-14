#define CINMIX_STORAGE_TEST
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../src/storage.c"
#include "../src/pe.h"
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr,"FAIL pe_disk:%d: %s\n",__LINE__,#condition);exit(1); } } while(0)
static FILE *disk;
int storage_test_read(uint32_t lba, uint8_t *buffer) { return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buffer,1,512,disk)==512; }
int storage_test_write(uint32_t lba, const uint8_t *buffer) { return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buffer,1,512,disk)==512 && fflush(disk)==0; }
static uint32_t resolve(const char *lib, const char *name) {
    if (strcmp(lib,"KERNEL32.DLL")) return 0;
    if (!strcmp(name,"GetCurrentProcessId")) return CM_PE_BASE+CM_PE_THUNK_OFFSET;
    if (!strcmp(name,"GetTickCount")) return CM_PE_BASE+CM_PE_THUNK_OFFSET+16;
    if (!strcmp(name,"ExitProcess")) return CM_PE_BASE+CM_PE_THUNK_OFFSET+32;
    return 0;
}
int main(int argc, char **argv) {
    CHECK(argc==2);
    disk=fopen(argv[1],"r+b"); CHECK(disk);
    CHECK(storage_mount());
    CHECK(storage_slot("hello.exe")>=0);
    uint8_t data[4097];
    int size=storage_load("hello.exe",(char *)data,sizeof(data));
    CHECK(size==2048);
    uint8_t image[0x4000]={0}; CmPeImage result;
    CHECK(cm_pe_load(data,(uint32_t)size,image,sizeof(image),&result,resolve)==CM_PE_OK);
    CHECK(result.entry==CM_PE_BASE+0x1000 && result.import_count==3);
    CHECK(storage_save("keep.txt","persist",7));
    CHECK(storage_mount());
    CHECK(storage_load("hello.exe",(char *)data,sizeof(data))==size);
    CHECK(storage_load("keep.txt",(char *)data,sizeof(data))==7);
    CHECK(memcmp(data,"persist",7)==0);
    fclose(disk);
    puts("PASS: signed CinmixFS disk installs real PE32 EXE, persists across mount, API import resolution, and unrelated file saves");
    return 0;
}
