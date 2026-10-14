#define _GNU_SOURCE
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
#define ok(x) do { if (!(x)) { fprintf(stderr,"FAIL ui17 line %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void map_area(uintptr_t p) { ok(mmap((void*)p,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0) != MAP_FAILED); }
static void frame(void) { repaint(); ok(!memcmp(canvas,(void*)(uintptr_t)boot_fb,PIXELS*4)); }
static void frames(int n) { for(int i=0;i<n;i++) frame(); }
static void shot(const char *path) {
    FILE *f=fopen(path,"wb"); ok(f);
    fprintf(f,"P6\n1920 1080\n255\n");
    for(size_t i=0;i<PIXELS;i++) { uint32_t v=canvas[i]; fputc(v>>16&255,f); fputc(v>>8&255,f); fputc(v&255,f); }
    fclose(f);
}
static void click(int x, int y) {
    mouse_x=x; mouse_y=y;
    handle_mouse_byte(9);handle_mouse_byte(0);handle_mouse_byte(0);
    handle_mouse_byte(8);handle_mouse_byte(0);handle_mouse_byte(0);
}
static void type_word(const char *p) {
    while(*p) { int key=0;for(int j=1;j<128;j++) if(plain_keys[j]==*p) {key=j;break;} ok(key);handle_key((uint8_t)key);p++; }
}
int main(void) {
    map_area(0x400000);map_area(0xD00000);map_area(0x2000000);
    sw=1920;sh=1080;mouse_x=1650;mouse_y=950;
    for(int i=0;i<APPS;i++) {windows[i].w=1050;windows[i].h=700;windows[i].x=(sw-1050)/2+14*(i%4);windows[i].y=(sh-700)/2-12+15*(i%4);}
    copy(clock_date,"2026-10-14",11);copy(clock_time,"21:18:00",9);
    frame();open_window(FILES);frames(WINDOW_OPEN_STEPS+2);
    Window *w=&windows[FILES];int x=w->x+2,y=w->y+34;
    ok(files_count()==5);
    click(x+50,y+27);ok(file_created==1&&files_count()==6&&selected_file==5);
    click(x+117,y+27);ok(file_sort==1&&files_item(0)==5);
    click(x+185,y+27);ok(file_view==1&&files_row_height()==66);
    click(x+235,y+27);ok(!file_details&&!files_preview_width(w->w-4));
    click(x+235,y+27);ok(file_details);
    click(x+75,y+128+5*36+12);ok(file_location==5&&files_visible_count()==1&&files_item(0)==3);
    click(x+75,y+128+3*36+12);ok(file_location==3&&files_visible_count()>=3);
    click(x+45,y+65);ok(file_location==6);
    click(x+w->w-4-130,y+65);ok(file_search_focus);
    type_word("notes");ok(file_search_len==5&&files_visible_count()==1&&files_item(0)==2);
    frame();shot("build/explorer-search-17.ppm");
    handle_key(0x1c);ok(!file_search_focus);
    click(files_list_left(x,w->w-4)+60,files_row_top(y)+12);ok(selected_file==2);
    handle_key(0x1c);ok(windows[NOTES].opened);
    open_window(FILES);frames(WINDOW_OPEN_STEPS+2);
    click(x+w->w-4-130,y+65);ok(file_search_focus);
    for(int i=0;i<5;i++)handle_key(0x0e);
    handle_key(0x01);ok(!file_search_focus&&file_search_len==0);
    file_sort=0;file_view=0;file_location=6;dirty=1;frame();shot("build/explorer-17.ppm");
    open_window(APPEAR);frames(WINDOW_OPEN_STEPS+2);
    Window *s=&windows[APPEAR];int sx=s->x+2,sy=s->y+34,nw=210,mx=sx+nw+34,mw=s->w-4-nw-55;
    ok(settings_page==1);
    click(sx+76,sy+132);ok(settings_page==0);
    int oldvolume=volume_level;click(mx+110,sy+304);ok(volume_level==oldvolume+10);
    frame();shot("build/settings-system-17.ppm");
    click(sx+76,sy+179);ok(settings_page==1);
    click(mx+mw-80,sy+129);ok(NIGHT);
    click(mx+145+25,sy+245);ok(!NIGHT&&theme==1&&previous_theme==1);
    click(mx+mw-80,sy+129);ok(NIGHT&&previous_theme==1);
    click(mx+2*145+20,sy+351);ok(wallpaper==2);
    click(mx+155+80+20,sy+421);ok(glass_level==1);
    click(mx+155+2*65+20,sy+474);ok(accent_index==2);
    frame();shot("build/settings-dark-17.ppm");
    click(sx+76,sy+226);ok(settings_page==2);
    frame();shot("build/settings-apps-17.ppm");
    click(mx+mw-53,sy+95+8*45+19);ok(windows[CALENDAR].opened);
    open_window(APPEAR);frames(WINDOW_OPEN_STEPS+1);
    click(sx+76,sy+273);ok(settings_page==3);
    click(mx+mw-60,sy+129);ok(do_not_disturb);
    notify("Settings test event");ok(notification_count>0);
    click(mx+mw-80,sy+244);ok(notification_count==0);
    click(sx+76,sy+320);ok(settings_page==4);
    click(sx+60,sy+75);ok(settings_search_focus);
    type_word("theme");handle_key(0x1c);ok(settings_page==1&&!settings_search_focus);
    click(mx+mw-80,sy+129);ok(!NIGHT&&previous_theme==1);
    frame();shot("build/settings-light-17.ppm");
    click(s->x+s->w-18,s->y+16);ok(!s->opened);
    printf("PASS: Explorer New/Sort/View/Details/nav/breadcrumb/search/row/Enter and Windows-style Settings category/page/theme/wallpaper/glass/accent/system volume/apps/notifications/search, PS2 mouse interaction, 1080p frame consistency\n");
    return 0;
}
