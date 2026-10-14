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
#define check(x) do { if (!(x)) { fprintf(stderr,"FAIL ui18 line %d: %s\n",__LINE__,#x);exit(1); } } while(0)
static FILE *disk;
int storage_test_read(uint32_t lba, uint8_t *buffer) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buffer,1,512,disk)==512;
}
int storage_test_write(uint32_t lba, const uint8_t *buffer) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buffer,1,512,disk)==512 && fflush(disk)==0;
}
static void map_area(uintptr_t addr) {
    check(mmap((void*)addr,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
}
static void frame(void) { repaint();check(memcmp(canvas,(void*)(uintptr_t)boot_fb,PIXELS*4)==0); }
static void frames(int n) { for(int i=0;i<n;i++)frame(); }
static void screenshot(const char *path) {
    FILE *f=fopen(path,"wb");check(f);
    fprintf(f,"P6\n1920 1080\n255\n");
    for(size_t i=0;i<PIXELS;i++) {uint32_t c=canvas[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f);}
    fclose(f);
}
static void click(int x,int y) {
    mouse_x=x;mouse_y=y;
    handle_mouse_byte(9);handle_mouse_byte(0);handle_mouse_byte(0);
    handle_mouse_byte(8);handle_mouse_byte(0);handle_mouse_byte(0);
}
int main(int argc,char **argv) {
    check(argc==2);
    disk=fopen(argv[1],"r+b");check(disk);
    map_area(0x400000);map_area(0xd00000);map_area(0x2000000);
    sw=1920;sh=1080;mouse_x=1500;mouse_y=850;
    for(int i=0;i<APPS;i++) {windows[i].w=1050;windows[i].h=700;windows[i].x=(sw-1050)/2+14*(i%4);windows[i].y=(sh-700)/2-12+15*(i%4);}
    check(storage_mount());
    notes_select("notes.txt");files_restore();
    check(notes_len==0 && file_created==0);
    open_window(NOTES);frames(WINDOW_OPEN_STEPS+1);
    handle_key(0x23);handle_key(0x12);handle_key(0x26);handle_key(0x26);handle_key(0x18);
    check(notes_dirty && !strcmp(notes,"hello"));
    handle_key(0x1d);handle_key(0x1f);handle_key(0x9d);
    check(!notes_dirty && storage_ready && !notes_save_failed);
    open_window(FILES);frames(WINDOW_OPEN_STEPS+1);
    Window *w=&windows[FILES];
    click(w->x+2+50,w->y+34+27);
    check(file_created==1 && selected_file==FILE_COUNT);
    check(storage_slot("New document.txt")>=0);
    int x=w->x+2, y=w->y+34;
    mouse_x=files_list_left(x,w->w-4)+54;
    mouse_y=files_row_top(y)+FILE_COUNT*files_row_height()+12;
    file_location=6;file_sort=0;file_view=0;
    click(mouse_x,mouse_y);
    check(selected_file==FILE_COUNT);
    handle_key(0x1c);
    check(active_window()==NOTES);
    check(!strcmp(notes_filename,"New document.txt"));
    handle_key(0x17);handle_key(0x18);
    check(!strcmp(notes,"io"));
    close_window(NOTES,0);frames(WINDOW_CLOSE_STEPS+1);
    check(!notes_dirty);
    frame();screenshot("build/persistent-explorer-18.ppm");
    storage_ready=0;file_created=0;notes_len=0;notes[0]=0;notes_dirty=0;
    check(storage_mount());files_restore();
    check(file_created==1 && !strcmp(file_extra_names[0],"New document.txt"));
    notes_select("notes.txt");check(!strcmp(notes,"hello"));
    notes_select("New document.txt");check(!strcmp(notes,"io"));
    open_window(NOTES);frames(WINDOW_OPEN_STEPS+1);
    frame();screenshot("build/persistent-notes-18.ppm");
    printf("PASS: 1080p Explorer create persisted, Notes Ctrl+S and save-on-close, storage remount restores both documents, composited display consistent\n");
    check(fclose(disk)==0);
    return 0;
}
