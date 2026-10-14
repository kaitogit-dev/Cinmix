#define _GNU_SOURCE
#define CINMIX_STORAGE_TEST
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include "../src/kernel.c"
uint32_t boot_fb = 0x2000000;
uint16_t boot_pitch = 7680;
uint16_t boot_width = 1920;
uint16_t boot_height = 1080;
uint8_t boot_bpp = 32;
#define PIXELS (1920u * 1080u)
#define check(ok) do { if (!(ok)) { fprintf(stderr,"FAIL ui20 line %d: %s\n",__LINE__,#ok);exit(1); } } while(0)
static FILE *disk;
int storage_test_read(uint32_t lba,uint8_t *buf) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buf,1,512,disk)==512;
}
int storage_test_write(uint32_t lba,const uint8_t *buf) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buf,1,512,disk)==512 && fflush(disk)==0;
}
static void map_area(uintptr_t a) {
    check(mmap((void *)a,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
}
static void frame(void) {
    repaint(); cm_pump(CM_QUEUE_LIMIT,cm_ui_dispatch);
    check(memcmp(canvas,(void *)(uintptr_t)boot_fb,PIXELS*4)==0);
}
static void frames(int count) { for(int i=0;i<count;i++) frame(); }
static void shot(const char *path) {
    FILE *f=fopen(path,"wb");check(f);
    fprintf(f,"P6\n1920 1080\n255\n");
    for(size_t i=0;i<PIXELS;i++) { uint32_t c=canvas[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f); }
    fclose(f);
}
int main(int argc,char **argv) {
    check(argc==2);
    disk=fopen(argv[1],"r+b");check(disk);
    map_area(0x400000); map_area(0xd00000); map_area(0x2000000);
    sw=1920; sh=1080; mouse_x=1350; mouse_y=830;
    for(int i=0;i<APPS;i++) {windows[i].w=1050;windows[i].h=700;windows[i].x=(sw-1050)/2+14*(i%4);windows[i].y=(sh-700)/2-12+15*(i%4);}
    check(storage_mount());
    check(NT_SUCCESS(NtInitializeIoManager()));
    nt_registry_load();
    check(nt_registry_loaded);
    uint32_t before=nt_irp_total;
    select_theme(3);
    check(theme==3 && nt_irp_total>before);
    int config_theme=0;
    check(nt_restore_theme(&config_theme) && config_theme==3);
    open_window(NOTES); frames(WINDOW_OPEN_STEPS+1);
    handle_key(0x31); handle_key(0x14);
    check(!strcmp(notes,"nt") && notes_dirty);
    handle_key(0x1d); handle_key(0x1f); handle_key(0x9d);
    check(!notes_dirty && storage_slot("notes.txt")>=0);
    open_window(FILES);frames(WINDOW_OPEN_STEPS+2);
    Window *w=&windows[FILES];
    mouse_x=w->x+2+50; mouse_y=w->y+34+27;
    handle_mouse_byte(9);handle_mouse_byte(0);handle_mouse_byte(0);
    handle_mouse_byte(8);handle_mouse_byte(0);handle_mouse_byte(0);
    check(file_created==1 && storage_slot("New document.txt")>=0);
    check(nt_open_handle_count()==0);
    open_window(SYSTEM);frames(WINDOW_OPEN_STEPS+1);
    shot("build/nt-system-20.ppm");
    open_window(CONSOLE);frames(WINDOW_OPEN_STEPS+1);
    copy(console_input,"handles",80);console_len=7;execute_command();
    int found=0; for(int i=0;i<log_count;i++) if(begins(log_lines[i],"Kernel handles in use")) found=1;
    check(found);
    storage_ready=0; notes[0]=0;notes_len=0;file_created=0;
    check(storage_mount());nt_registry_load();
    check(nt_restore_theme(&config_theme) && config_theme==3);
    notes_select("notes.txt");check(!strcmp(notes,"nt"));
    files_restore();check(file_created==1);
    check(nt_irp_total>10 && nt_disk_driver.DeviceCount==1);
    printf("PASS: UI Notes/Explorer route through Nt* calls and IRPs, persistent Theme registry, handles diagnostics, 1080p framebuffer and remount\n");
    fclose(disk); return 0;
}
