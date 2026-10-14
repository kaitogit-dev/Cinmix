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
#define check(ok) do { if (!(ok)) { fprintf(stderr,"FAIL ui131 line %d: %s\n",__LINE__,#ok); exit(1); } } while(0)
int main(void) {
    check(mmap((void*)0x400000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0xD00000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    check(mmap((void*)0x2000000,1280*720*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
    sw=1280; sh=720; mouse_x=1100; mouse_y=600;
    for(int i=0;i<APPS;i++) { windows[i].x=260+14*(i%4); windows[i].y=87+15*(i%4); windows[i].w=760; windows[i].h=485; }
    int x0,y0,x1,y1,x4,y4;
    desktop_position(0,&x0,&y0); desktop_position(1,&x1,&y1); desktop_position(4,&x4,&y4);
    check(y1-y0==76 && x4-x0==96);
    repaint();
    int sx=x0+25, sy=y0+27;
    check(canvas[(size_t)sy*sw+sx] != desktop_cache[(size_t)sy*sw+sx]);
    int w,h;
    tooltip_size(FILES,0,&w,&h);
    check(w>=92 && h==36);
    int items[APPS]; int count=taskbar_items(items);
    int bar=taskbar_left(count);
    check(tooltip_left(FILES,w)+w/2==bar+taskbar_width()/2);
    check(tooltip_top(h)==sh-62-h);
    tooltip_size(FILES,1,&w,&h);
    check(w==202 && h==152);
    hover_visible=1; task_hover=FILES;
    repaint();
    int label_x=tooltip_left(FILES,92);
    check(canvas[(size_t)(tooltip_top(36)+17)*sw+label_x+6] != 0);
    open_window(FILES);
    for(int i=0;i<8;i++) repaint();
    check(windows[FILES].opened && !windows[FILES].minimized);
    close_window(FILES,1);
    check(windows[FILES].minimized);
    open_window(FILES);
    check(!windows[FILES].minimized);
    printf("PASS: denser 96x76 shortcut grid, enlarged icon art, light 36px tooltip anchored to taskbar icon, running preview layout and minimize/restore\n");
    return 0;
}
