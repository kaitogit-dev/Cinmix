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
#define check(c) do { if (!(c)) {fprintf(stderr,"FAIL ui22 line %d: %s\n",__LINE__,#c);exit(1);} } while(0)
static FILE *disk;
int storage_test_read(uint32_t lba, uint8_t *buffer) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buffer,1,512,disk)==512;
}
int storage_test_write(uint32_t lba, const uint8_t *buffer) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buffer,1,512,disk)==512 && fflush(disk)==0;
}
static void frame(void) { repaint();check(!memcmp(canvas,(void *)(uintptr_t)boot_fb,PIXELS*4)); }
static void frames(int n) {for(int i=0;i<n;i++) frame();}
static void screenshot(const char *name) {
    FILE *f=fopen(name,"wb");check(f);
    fprintf(f,"P6\n1920 1080\n255\n");
    for(size_t i=0;i<PIXELS;i++){uint32_t c=canvas[i];fputc(c>>16&255,f);fputc(c>>8&255,f);fputc(c&255,f);}
    fclose(f);
}
static void click_at(int x,int y) {
    mouse_x=x;mouse_y=y;
    handle_mouse_byte(9);handle_mouse_byte(0);handle_mouse_byte(0);
    handle_mouse_byte(8);handle_mouse_byte(0);handle_mouse_byte(0);
}
int main(int argc,char **argv) {
    check(argc==3);
    disk=fopen(argv[1],"r+b");check(disk);
    check(mmap((void*)0x400000,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0xD00000,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0x2000000,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    sw=1920;sh=1080;mouse_x=1600;mouse_y=830;
    for(int i=0;i<APPS;i++) {windows[i].w=1050;windows[i].h=700;windows[i].x=(sw-1050)/2+14*(i%4);windows[i].y=(sh-700)/2-12+15*(i%4);}
    check(storage_mount());
    check(NT_SUCCESS(NtInitializeIoManager()));
    if (!strcmp(argv[2],"save")) {
        open_window(SKETCH);frames(WINDOW_OPEN_STEPS+1);
        check(!strcmp(app_name[SKETCH],"Pinta"));
        Window *w=&windows[SKETCH];
        int tx=w->x+2+w->w-4-247, ty=w->y+34;
        click_at(tx+22,ty+68+0*44+16);
        check(sketch_tool==PINTA_PENCIL);
        click_at(w->x+20+20*3,w->y+34+44+20*3);
        check(sketch_pixels[20][20]==sketch_color && sketch_has_undo);
        click_at(tx+18,ty+68+2*44+18);
        check(sketch_tool==PINTA_RECTANGLE);
        mouse_x=w->x+20+30*3; mouse_y=w->y+34+44+30*3;
        handle_mouse_byte(9);handle_mouse_byte(0);handle_mouse_byte(0);
        mouse_x=w->x+20+70*3; mouse_y=w->y+34+44+70*3;
        handle_mouse_byte(8);handle_mouse_byte(0);handle_mouse_byte(0);
        check(sketch_pixels[30][30]==sketch_color && sketch_pixels[70][70]==sketch_color);
        click_at(tx+118+22,ty+68+2*44+18);
        check(sketch_tool==PINTA_FILL);
        sketch_color=5;
        click_at(w->x+20+50*3,w->y+34+44+50*3);
        check(sketch_pixels[50][50]==5);
        undo_sketch();check(sketch_pixels[50][50]==0);
        redo_sketch();check(sketch_pixels[50][50]==5);
        sketch_color=3;
        sketch_tool=PINTA_BRUSH;
        click_at(w->x+20+10*3,w->y+34+44+10*3);
        check(sketch_pixels[10][10]==3);
        frame();screenshot("build/pinta-22.ppm");
        handle_key(0x1d);handle_key(0x1f);handle_key(0x9d);
        check(!sketch_dirty && sketch_saved && storage_slot("painting0.cmx")>=0 && storage_slot("painting6.cmx")>=0);
        check(nt_open_handle_count()==0);
        puts("PASS: Pinta native toolbar, pencil/rect/fill, undo-redo, Ctrl+S, seven NT file segments saved on disk");
    } else if (!strcmp(argv[2],"reload")) {
        check(pinta_load_document());
        check(sketch_pixels[20][20]==1 && sketch_pixels[50][50]==5 && sketch_pixels[70][70]==1);
        check(sketch_pixels[10][10]==3);
        open_window(SKETCH);frames(WINDOW_OPEN_STEPS+1);
        frame();screenshot("build/pinta-reload-22.ppm");
        puts("PASS: Pinta painting survives another process, NT segmented file read, 1920x1080 full-frame compositing");
    } else check(0);
    check(fclose(disk)==0);
    return 0;
}
