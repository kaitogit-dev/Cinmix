#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include "../src/kernel.c"
uint32_t boot_fb = 0x2000000;
uint16_t boot_pitch = 5120;
uint16_t boot_width = 1280;
uint16_t boot_height = 720;
uint8_t boot_bpp = 32;
#define require(ok) do { if (!(ok)) { fprintf(stderr, "FAIL perf_aa line %d: %s\n", __LINE__, #ok); exit(1); } } while(0)
static void *map(int addr) {
    void *p = mmap((void *)(uintptr_t)addr, 1280*720*4, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,-1,0);
    require(p != MAP_FAILED);
    return p;
}
static double time_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)/1000000.0;
}
static uint32_t *snapshot;
static void equal_to_full(const char *label) {
    require(!dirty);
    memcpy(snapshot, canvas, (size_t)sw*sh*4);
    repaint();
    int different=0;
    for (int i=0;i<sw*sh;i++) if (snapshot[i]!=canvas[i]) different++;
    if (different) {
        int x0=sw,y0=sh,x1=0,y1=0;
        for(int y=0;y<sh;y++) for(int x=0;x<sw;x++) if(snapshot[(size_t)y*sw+x]!=canvas[(size_t)y*sw+x]) {
            if(x<x0)x0=x; if(y<y0)y0=y; if(x>x1)x1=x; if(y>y1)y1=y;
        }
        fprintf(stderr,"DIFFERENCE %s: %d pixels bbox (%d,%d) to (%d,%d) sample %#x to %#x\n",label,different,x0,y0,x1,y1,snapshot[(size_t)y0*sw+x0],canvas[(size_t)y0*sw+x0]);
        FILE *f=fopen("build/damage-before.ppm","wb"); fprintf(f,"P6\n%d %d\n255\n",sw,sh);
        for(int i=0;i<sw*sh;i++){ fputc((snapshot[i]>>16)&255,f); fputc((snapshot[i]>>8)&255,f); fputc(snapshot[i]&255,f); } fclose(f);
        f=fopen("build/damage-after.ppm","wb"); fprintf(f,"P6\n%d %d\n255\n",sw,sh);
        for(int i=0;i<sw*sh;i++){ fputc((canvas[i]>>16)&255,f); fputc((canvas[i]>>8)&255,f); fputc(canvas[i]&255,f); } fclose(f);
        exit(1);
    }
}
static void frames(int n) { for(int i=0;i<n;i++) repaint(); }
int main(void) {
    map(0x400000); map(0x900000); map(0xd00000); map(0x2000000);
    snapshot=malloc(1280*720*4); require(snapshot);
    sw=1280; sh=720;
    copy(clock_time,"15:39:00",9); copy(clock_date,"2026-10-14",11);
    for(int i=0;i<APPS;i++) { windows[i].x=260+14*(i%4); windows[i].y=87+15*(i%4); windows[i].w=760; windows[i].h=485; }
    mouse_x=1100; mouse_y=600;
    repaint();
    int softened=0;
    clip_full(); rect(10,10,100,100,0x224466);
    rounded(10,10,100,100,16,0xffffff);
    for(int y=10;y<26;y++) for(int x=10;x<26;x++) {
        uint32_t c=canvas[(size_t)y*sw+x];
        if(c!=0x224466 && c!=0xffffff) softened++;
    }
    require(softened>0);
    rect(10,10,100,100,0x224466);
    line(10,10,69,48,0xffffff);
    softened=0;
    for(int y=10;y<=50;y++) for(int x=10;x<=69;x++) {
        uint32_t c=canvas[(size_t)y*sw+x];
        if(c!=0x224466 && c!=0xffffff) softened++;
    }
    require(softened>0);
    dirty=0; repaint();
    struct timespec start,end;
    uint32_t before=presented_pixels;
    clock_gettime(CLOCK_MONOTONIC,&start);
    open_window(FILES);
    frames(7);
    clock_gettime(CLOCK_MONOTONIC,&end);
    uint32_t opened=presented_pixels-before;
    printf("open_7_frames_ms %.3f\n",time_ms(start,end));
    printf("open_7_frames_pixels %u\n",opened);
    require(opened < 7u*1280*720*7/10);
    require(damage_frames>=7);
    frames(1);
    equal_to_full("files open settled");
    dirty=1; repaint();
    equal_to_full("files active");
    open_window(NOTES);
    frames(8);
    equal_to_full("two overlapping windows");
    close_window(NOTES,1);
    frames(8);
    equal_to_full("minimize with overlap");
    open_window(NOTES);
    frames(8);
    equal_to_full("restore overlap");
    toggle_maximize(NOTES);
    frames(8);
    equal_to_full("maximize");
    toggle_maximize(NOTES);
    frames(8);
    equal_to_full("unmaximize");
    for(int i=0;i<APPS;i++) { windows[i].opened=0; windows[i].minimized=0; anim_visible[i]=0; }
    dirty=1; repaint();
    equal_to_full("closed windows");
    start_open=1; start_anim=8; dirty=1;
    before=presented_pixels;
    clock_gettime(CLOCK_MONOTONIC,&start);
    frames(8);
    clock_gettime(CLOCK_MONOTONIC,&end);
    printf("start_8_frames_ms %.3f\n",time_ms(start,end));
    printf("start_8_frames_pixels %u\n",presented_pixels-before);
    require(presented_pixels-before < 8u*1280*720*8/10);
    equal_to_full("start menu");
    start_open=0; dirty=1; repaint();
    equal_to_full("closed menu");
    notification_open=1; notification_anim=8; dirty=1;
    frames(8);
    equal_to_full("notification panel");
    notification_open=0; dirty=1; repaint();
    equal_to_full("closed notifications");
    task_hover=FILES; mouse_x=taskbar_left(7)+20; mouse_y=sh-27; dirty=1;
    repaint();
    equal_to_full("taskbar tooltip");
    wallpaper=1; dirty=1; repaint();
    equal_to_full("wallpaper invalidation");
    printf("PASS: MSAA corners, alpha-blended line edges, damage-repaint reduction, incremental/full pixel parity\n");
    free(snapshot);
    return 0;
}
