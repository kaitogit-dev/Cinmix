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
#define REQUIRE(ok) do { if (!(ok)) { fprintf(stderr, "FAIL ui16 line %d: %s\n", __LINE__, #ok); exit(1); } } while (0)
#define PIXELS (1920u * 1080u)
static void map_region(uintptr_t addr) {
    REQUIRE(mmap((void *)addr, PIXELS * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) != MAP_FAILED);
}
static void frame(void) {
    repaint();
    REQUIRE(memcmp(canvas, (void *)(uintptr_t)boot_fb, PIXELS * 4) == 0);
}
static void frames(int count) { for (int i = 0; i < count; i++) frame(); }
static void capture(const char *name) {
    FILE *out = fopen(name, "wb"); REQUIRE(out != NULL);
    fprintf(out, "P6\n1920 1080\n255\n");
    for (size_t i = 0; i < PIXELS; i++) {
        uint32_t c = canvas[i];
        fputc((c >> 16) & 255, out);
        fputc((c >> 8) & 255, out);
        fputc(c & 255, out);
    }
    fclose(out);
}
static void click(int x, int y) { mouse_x=x; mouse_y=y; handle_click(); }
static void shortcut(int key) {
    handle_key(0x1d);
    handle_key(0x38);
    handle_key((uint8_t)key);
    handle_key(0xb8);
    handle_key(0x9d);
}
int main(void) {
    map_region(0x400000); map_region(0xD00000); map_region(0x2000000);
    sw=1920; sh=1080; mouse_x=1640; mouse_y=920;
    copy(clock_time, "20:42:15", 9);
    copy(clock_date, "2026-10-14", 11);
    for (int i=0; i<APPS; i++) {
        windows[i].w=1050; windows[i].h=700;
        windows[i].x=(sw-1050)/2+14*(i%4);
        windows[i].y=(sh-700)/2-12+15*(i%4);
    }
    frame();
    uint32_t light_background=canvas[(size_t)700*sw+1700];
    REQUIRE(!NIGHT && theme==0 && previous_theme==0 && !do_not_disturb);
    open_window(APPEAR); frames(WINDOW_OPEN_STEPS+2);
    Window *a=&windows[APPEAR];
    click(a->x+a->w-209+90, a->y+34+8+18);
    REQUIRE(theme==3 && previous_theme==0);
    frames(2);
    REQUIRE(cached_wallpaper_night == 1);
    REQUIRE(canvas[(size_t)700*sw+1700] != light_background);
    REQUIRE(INK != PAPER && PAPER==0x182637 && UI_FIELD!=0xf8fcff);
    capture("build/appearance-16-dark.ppm");
    open_window(FILES); frames(WINDOW_OPEN_STEPS+2);
    REQUIRE(windows[FILES].opened && !windows[FILES].minimized);
    mouse_x = files_list_left(windows[FILES].x + 2, windows[FILES].w - 4) + 18;
    mouse_y = files_row_top(windows[FILES].y + 34) + files_row_height() + 10;
    REQUIRE(files_row_hit(&windows[FILES], mouse_x, mouse_y) == 1);
    handle_click();
    REQUIRE(selected_file == 1);
    selected_file = 3; dirty = 1; frame();
    capture("build/files-16.ppm");
    quick_open=1; dirty=1; frames(PANEL_STEPS+1);
    capture("build/quick-16.ppm");
    int qx=sw-quick_width()-16, qy=sh-60-quick_height();
    click(qx+110,qy+386);
    REQUIRE(do_not_disturb && !notification_unread);
    int old_count=notification_count;
    notify("Dark Aero quiet event");
    REQUIRE(notification_count==old_count+1 && notification_unread==0);
    quick_open=0; notification_open=1; dirty=1; frames(PANEL_STEPS+1);
    capture("build/notifications-16.ppm");
    notification_open=0; dirty=1; frames(PANEL_STEPS+1);
    shortcut(0x32);
    REQUIRE(theme==0 && !NIGHT && previous_theme==0);
    frames(2);
    shortcut(0x32);
    REQUIRE(theme==3 && NIGHT);
    frames(2);
    run_open=1; copy(run_input,"light",40); run_len=5;
    run_command();
    REQUIRE(!run_open && theme==0 && previous_theme==0);
    frame();
    select_theme(1);
    REQUIRE(theme==1 && previous_theme==1);
    toggle_dark_aero(); REQUIRE(theme==3 && previous_theme==1);
    toggle_dark_aero(); REQUIRE(theme==1);
    mouse_x=1650; mouse_y=500;
    handle_right_click(); REQUIRE(context_open);
    frames(PANEL_STEPS+1);
    click(context_x+40, context_y+13+4*38+10);
    REQUIRE(theme==3 && !context_open);
    frames(2);
    quick_open=1; frames(PANEL_STEPS+1);
    click(qx+100,qy+356);
    REQUIRE(theme==1 && !NIGHT);
    click(qx+100,qy+356);
    REQUIRE(theme==3);
    click(qx+100,qy+386);
    REQUIRE(!do_not_disturb);
    quick_open=0; frames(PANEL_STEPS+1);
    int app=active_window();
    REQUIRE(app==FILES);
    shortcut(0x48);
    REQUIRE(windows[FILES].maximized);
    frames(WINDOW_RESIZE_STEPS+1);
    shortcut(0x50);
    REQUIRE(windows[FILES].minimized && anim_exit[FILES]);
    frames(WINDOW_CLOSE_STEPS+1);
    open_window(FILES); frames(WINDOW_OPEN_STEPS+1);
    REQUIRE(!windows[FILES].minimized);
    close_window(FILES,0); frames(WINDOW_CLOSE_STEPS+1);
    mouse_x = 52; mouse_y = 58;
    desktop_hover = desktop_slot_hit(mouse_x, mouse_y);
    REQUIRE(desktop_hover == 0);
    dirty = 1; frame();
    capture("build/desktop-hover-16.ppm");
    start_open=1; dirty=1; frames(PANEL_STEPS+1);
    capture("build/start-16.ppm");
    start_open=0; frames(PANEL_STEPS+1);
    REQUIRE(!panel_visible(PANEL_START));
    printf("PASS: 1920x1080 Dark Aero plus Explorer shell, desktop hover highlights, theme switching, quiet notifications and framebuffer consistency\n");
    return 0;
}
