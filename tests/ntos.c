#define _GNU_SOURCE
#define CINMIX_STORAGE_TEST
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/storage.c"
#include "../src/objects.c"
#include "../src/ntos.c"

static FILE *disk;
static int fail_commit;
static unsigned writes;
#define check(ok) do { if (!(ok)) { fprintf(stderr, "FAIL ntos line %d: %s\n", __LINE__, #ok); exit(1); } } while (0)
int storage_test_read(uint32_t lba, uint8_t *buf) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buf,1,512,disk)==512;
}
int storage_test_write(uint32_t lba,const uint8_t *buf) {
    writes++;
    if (fail_commit && (lba==1 || lba==2)) return 0;
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buf,1,512,disk)==512 && fflush(disk)==0;
}
static HANDLE create_path(const char *ascii, uint32_t access, uint32_t disposition, NTSTATUS *out) {
    uint16_t path[96]; UNICODE_STRING us;
    check(RtlInitUnicodeStringFromAscii(&us,path,96,ascii));
    OBJECT_ATTRIBUTES oa={sizeof(oa),0,&us,0,NULL,NULL};
    IO_STATUS_BLOCK ios={0};
    HANDLE h=0;
    *out=NtCreateFile(&h,access,&oa,&ios,NULL,0,0,disposition,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
    check(*out==ios.Status);
    return h;
}
static HANDLE open_key(void) {
    uint16_t chars[96]; UNICODE_STRING us;
    check(RtlInitUnicodeStringFromAscii(&us,chars,96,"\\Registry\\Machine\\Software\\Cinmix"));
    OBJECT_ATTRIBUTES oa={sizeof(oa),0,&us,0,NULL,NULL};
    HANDLE h=0;
    check(NT_SUCCESS(NtOpenKey(&h,3,&oa)));
    return h;
}
static void set_theme(uint32_t value) {
    HANDLE k=open_key();
    uint16_t chars[16]; UNICODE_STRING name;
    check(RtlInitUnicodeStringFromAscii(&name,chars,16,"Theme"));
    check(NT_SUCCESS(NtSetValueKey(k,&name,0,REG_DWORD,&value,4)));
    check(NtCloseObject(k)==STATUS_SUCCESS);
}
static uint32_t query_theme(void) {
    HANDLE k=open_key();
    uint16_t chars[16]; UNICODE_STRING name;
    check(RtlInitUnicodeStringFromAscii(&name,chars,16,"Theme"));
    uint8_t payload[16]; uint32_t length=0;
    check(NtQueryValueKey(k,&name,2,payload,15,&length)==STATUS_BUFFER_TOO_SMALL && length==16);
    check(NT_SUCCESS(NtQueryValueKey(k,&name,2,payload,sizeof(payload),&length)));
    check(NtCloseObject(k)==STATUS_SUCCESS);
    return (uint32_t)payload[12] | ((uint32_t)payload[13]<<8) | ((uint32_t)payload[14]<<16) | ((uint32_t)payload[15]<<24);
}
int main(int argc,char **argv) {
    check(argc==3);
    disk=fopen(argv[1],"r+b");check(disk);
    check(storage_mount());
    check(NT_SUCCESS(NtInitializeIoManager()));
    check(nt_name_equal(nt_disk_device.DeviceName,"\\Device\\CinmixDisk"));
    check(nt_name_equal(nt_dos_link,"\\??\\D:"));
    check(nt_disk_driver.DeviceCount==1);
    IO_STATUS_BLOCK io={0}; NTSTATUS s;
    char buffer[64]={0};
    if(!strcmp(argv[2],"save")) {
        check(!create_path("\\??\\C:\\unsafe.txt",FILE_WRITE_DATA,FILE_OPEN_IF,&s));
        check(s==STATUS_OBJECT_PATH_NOT_FOUND);
        check(!create_path("\\??\\D:\\..\\unsafe",FILE_WRITE_DATA,FILE_OPEN_IF,&s));
        check(s==STATUS_OBJECT_NAME_NOT_FOUND);
        check(!create_path("\\??\\D:\\absent.txt",FILE_READ_DATA,FILE_OPEN,&s));
        check(s==STATUS_OBJECT_NAME_NOT_FOUND);
        HANDLE h=create_path("\\??\\D:\\ntfile.txt",FILE_READ_DATA|FILE_WRITE_DATA,FILE_CREATE,&s);
        check(NT_SUCCESS(s) && h);
        check(!create_path("\\??\\D:\\ntfile.txt",FILE_WRITE_DATA,FILE_CREATE,&s));
        check(s==STATUS_OBJECT_NAME_COLLISION);
        FILE_STANDARD_INFORMATION info={0};
        check(NT_SUCCESS(NtQueryInformationFile(h,&io,&info,sizeof(info),NT_FILE_STANDARD_INFORMATION)));
        check(info.EndOfFile==0);
        check(NtWriteFile(h,0,NULL,NULL,&io,"NT kernel API",13,NULL,NULL)==STATUS_SUCCESS && io.Information==13);
        check(NtQueryInformationFile(h,&io,&info,sizeof(info),NT_FILE_STANDARD_INFORMATION)==STATUS_SUCCESS && info.EndOfFile==13);
        int64_t off=0;
        check(NtReadFile(h,0,NULL,NULL,&io,buffer,13,&off,NULL)==STATUS_SUCCESS && io.Information==13);
        check(!memcmp(buffer,"NT kernel API",13));
        check(NtFlushBuffersFile(h,&io)==STATUS_SUCCESS);
        check(NtClose(h)==STATUS_SUCCESS);
        check(NtClose(h)==STATUS_INVALID_HANDLE);
        HANDLE next=create_path("\\Device\\CinmixDisk\\ntfile.txt",FILE_READ_DATA,FILE_OPEN,&s);
        check(next && next!=h);
        check(NtReadFile(h,0,NULL,NULL,&io,buffer,13,NULL,NULL)==STATUS_INVALID_HANDLE);
        check(NtReadFile(next,0,NULL,NULL,&io,buffer,20,NULL,NULL)==STATUS_SUCCESS && io.Information==13);
        check(NtReadFile(next,0,NULL,NULL,&io,buffer,1,NULL,NULL)==STATUS_END_OF_FILE);
        check(NtClose(next)==STATUS_SUCCESS);
        set_theme(3);
        check(query_theme()==3);
        check(nt_irp_total>5 && nt_irp_failure>0);
        check(nt_open_handle_count()==0 && cm_handle_count()==0);
        printf("PASS: NT object namespace, IRP dispatch, file access, path checks, status codes, stale handles, registry commits\n");
    }else if(!strcmp(argv[2],"reload")) {
        nt_registry_load();
        check(nt_registry_loaded && query_theme()==3);
        HANDLE h=create_path("\\??\\D:\\ntfile.txt",FILE_READ_DATA,FILE_OPEN,&s);
        check(h);
        check(NtReadFile(h,0,NULL,NULL,&io,buffer,13,NULL,NULL)==STATUS_SUCCESS && !memcmp(buffer,"NT kernel API",13));
        check(NtClose(h)==STATUS_SUCCESS);
        set_theme(2);
        h=create_path("\\??\\D:\\ntfile.txt",FILE_WRITE_DATA,FILE_OVERWRITE_IF,&s);
        check(h);
        check(NtWriteFile(h,0,NULL,NULL,&io,"interrupted",11,NULL,NULL)==STATUS_SUCCESS);
        fail_commit=1;
        check(NtClose(h)==STATUS_IO_DEVICE_ERROR);
        check(NtClose(h)==STATUS_IO_DEVICE_ERROR);
        check(nt_file_abort(h)==STATUS_SUCCESS);
        fail_commit=0;
        check(storage_mount());
        nt_registry_load();
        check(query_theme()==2);
        puts("PASS: NT registry and file survive process restart; partial disk commit retains previous data");
    }else if(!strcmp(argv[2],"recover")) {
        nt_registry_load();
        check(query_theme()==2);
        HANDLE h=create_path("\\??\\D:\\ntfile.txt",FILE_READ_DATA,FILE_OPEN,&s);
        check(h);
        check(NtReadFile(h,0,NULL,NULL,&io,buffer,13,NULL,NULL)==STATUS_SUCCESS);
        check(!memcmp(buffer,"NT kernel API",13));
        check(NtClose(h)==STATUS_SUCCESS);
        check(nt_open_handle_count()==0);
        puts("PASS: NTAPI file data and registry retained after interrupted write and reboot");
    }else check(0);
    fclose(disk);
    return 0;
}
