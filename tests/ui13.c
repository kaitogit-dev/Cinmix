#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include "../src/kernel.c"
uint32_t boot_fb = 0x2000000;
uint16_t boot_pitch = 5120;
uint16_t boot_width = 1280;
uint16_t boot_height = 720;
uint8_t boot_bpp = 32;
#define check(ok) do { if (!(ok)) { fprintf(stderr, "FAIL ui13 line %d: %s\n", __LINE__, #ok); exit(1); } } while (0)
static void save(const char *name) {
    FILE *f=fopen(name,"wb"); check(f);
    fprintf(f,"P6\n%d %d\n255\n",sw,sh);
    for(int y=0;y<sh;y++) for(int x=0;x<sw;x++) {
        uint32_t c=canvas[(size_t)y*sw+x];
        fputc((c>>16)&255,f); fputc((c>>8)&255,f); fputc(c&255,f);
    }
    fclose(f);
}
static void frames(int count) { for(int i=0;i<count;i++) repaint(); }
int main(void) {
    check(mmap((void*)0x400000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0xD00000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0x2000000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    sw=1280;sh=720;mouse_x=600;mouse_y=350;
    copy(clock_time,"18:41:00",9);
    copy(clock_date,"2026-10-14",11);
    for(int i=0;i<APPS;i++) { windows[i].x=260+14*(i%4);windows[i].y=87+15*(i%4);windows[i].w=760;windows[i].h=485; }
    notify("Cinmix 0.13 is ready");
    frames(2);
    draw_pointer();
    check(pointer_saved);
    int changed=0;
    for(int y=0;y<CURSOR_H && !changed;y++) for(int x=0;x<CURSOR_W;x++) {
        int px=mouse_x+x, py=mouse_y+y;
        if ((unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh && canvas[(size_t)py*sw+px] != pointer_back[y][x]) { changed=1; break; }
    }
    check(changed);
    save("build/cursor-13.ppm");
    for(int y=0;y<CURSOR_H;y++) for(int x=0;x<CURSOR_W;x++) {
        int px=pointer_old_x+x, py=pointer_old_y+y;
        if ((unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh)
            canvas[(size_t)py*sw+px] = pointer_back[y][x];
    }
    pointer_saved=0;
    task_hover=FILES;
    mouse_x=taskbar_left(7)+20;
    mouse_y=sh-27;
    hover_delay=0;
    hover_visible=0;
    frames(1);
    check(!hover_visible);
    for(int i=0;i<hover_delay_target()-1;i++) update_hover_timer(1);
    check(!hover_visible);
    update_hover_timer(1);
    check(hover_visible);
    frames(2);
    save("build/tooltip-13.ppm");
    int before=notification_count;
    open_window(FILES);
    check(notification_count == before);
    handle_key(0x1d); handle_key(0x13);
    check(run_open);
    handle_key(0x9d);
    for(const char *p="notes";*p;p++) {
        char ch=*p;
        int key=0;
        for(int i=0;i<128;i++) if (plain_keys[i]==ch) { key=i; break; }
        check(key);
        handle_key((uint8_t)key);
    }
    check(same(run_input,"notes"));
    frames(2);
    save("build/run-13.ppm");
    handle_key(0x1c);
    check(!run_open && windows[NOTES].opened);
    printf("PASS: SVG cursor rendering, delayed taskbar tooltip timing, silent app launches, Ctrl+R Run dialog launch path\n");
    return 0;
}
