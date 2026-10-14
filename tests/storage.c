#define _GNU_SOURCE
#define CINMIX_STORAGE_TEST
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../src/storage.c"
static FILE *disk;
static int fail_metadata;
static int writes;
int storage_test_read(uint32_t lba, uint8_t *buf) {
    return fseek(disk, (long)lba * 512, SEEK_SET) == 0 && fread(buf, 1, 512, disk) == 512;
}
int storage_test_write(uint32_t lba, const uint8_t *buf) {
    writes++;
    if (fail_metadata && (lba == 1 || lba == 2)) return 0;
    return fseek(disk, (long)lba * 512, SEEK_SET) == 0 && fwrite(buf, 1, 512, disk) == 512 && fflush(disk) == 0;
}
#define check(x) do { if (!(x)) { fprintf(stderr,"FAIL storage line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(int argc, char **argv) {
    check(argc == 3);
    disk = fopen(argv[1], "r+b"); check(disk != NULL);
    char data[4100] = {0};
    if (!strcmp(argv[2], "save")) {
        check(storage_mount());
        check(storage_save("notes.txt", "Hello from Cinmix disk", 22));
        check(storage_save("New document.txt", "Hello persistent file", 21));
        check(storage_load("notes.txt", data, sizeof(data)) == 22);
        check(!memcmp(data,"Hello from Cinmix disk",22));
        check(storage_directory_lba == 1 || storage_directory_lba == 2);
        char name[20]={0};
        check(storage_list(0,name));
        check(!strcmp(name,"notes.txt"));
        printf("PASS: committed Notes and an Explorer document to the disk image\n");
    } else if (!strcmp(argv[2], "reload")) {
        check(storage_mount());
        check(storage_load("notes.txt",data,sizeof(data)) == 22);
        check(!memcmp(data,"Hello from Cinmix disk",22));
        check(storage_load("New document.txt",data,sizeof(data)) == 21);
        check(!memcmp(data,"Hello persistent file",21));
        check(storage_save("notes.txt", "Version two", 11));
        check(storage_load("notes.txt",data,sizeof(data)) == 11);
        check(!strcmp(data,"Version two"));
        fail_metadata=1;
        check(!storage_save("notes.txt", "Incomplete update", 17));
        fail_metadata=0;
        check(storage_mount());
        check(storage_load("notes.txt",data,sizeof(data)) == 11);
        check(!strcmp(data,"Version two"));
        printf("PASS: data survives new process and failed metadata commit preserves last saved version\n");
    } else if (!strcmp(argv[2], "recover")) {
        check(storage_mount());
        check(storage_load("notes.txt",data,sizeof(data)) == 11);
        check(!strcmp(data,"Version two"));
        int oldwrites=writes;
        check(!storage_save("../unsafe", "x",1));
        check(writes==oldwrites);
        printf("PASS: power-interrupted save rollback and unsafe path rejection\n");
    } else if (!strcmp(argv[2], "unsigned")) {
        check(!storage_mount());
        check(!storage_ready);
        check(!storage_save("notes.txt","data",4));
        check(writes==0);
        printf("PASS: refuses to write to an unformatted / unsigned disk\n");
    } else check(0);
    check(fclose(disk)==0);
    return 0;
}
