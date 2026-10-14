#include <stdint.h>
#include <stddef.h>

extern uint32_t boot_fb;
extern uint16_t boot_pitch;
extern uint16_t boot_width;
extern uint16_t boot_height;
extern uint8_t boot_bpp;
extern const uint8_t cinmix_font[96][16][8];

static int theme;
static int wallpaper;
static int glass_level = 2;
static int accent_index;
static const uint32_t accent_colors[4] = {0x2167b3,0x178f99,0x7852ba,0xcf7343};
static int desktop_hidden;
static int desktop_saved[11];
static const uint32_t palette[3][13] = {
    {0x243d60,0x537caa,0xa7bfd8,0xf9fcff,0xffffff,0x285f9a,0xd4e2ef,0xf2f7fc,0x2167b3,0x7ec5fc,0x10223b,0x3067a0,0xe5f0fc},
    {0x3a597c,0x6e93b4,0xb7cadf,0xf8fbfe,0xffffff,0x638eac,0xdbe5ed,0xfafcff,0x3d729f,0x9ed6fa,0x213b56,0x7797b8,0xeef4fa},
    {0x1d2b4a,0x435c86,0x8ea7cc,0xeef4fc,0xffffff,0x122c58,0xb5c5df,0xe3eaf7,0x4679c5,0x93b9fb,0x07152c,0x22416c,0xdce7fa}
};
#define INK palette[theme][0]
#define INK2 palette[theme][1]
#define STEEL palette[theme][2]
#define PAPER palette[theme][3]
#define WHITE palette[theme][4]
#define FLOOR palette[theme][5]
#define HAIR palette[theme][6]
#define LIGHT palette[theme][7]
#define BLUE (accent_index ? accent_colors[accent_index] : palette[theme][8])
#define ACTIVE palette[theme][9]
#define BLACK palette[theme][10]
#define DESK2 palette[theme][11]
#define TILE palette[theme][12]

#define FILES 0
#define NOTES 1
#define CONSOLE 2
#define SYSTEM 3
#define CLOCK 4
#define CALC 5
#define APPEAR 6
#define SKETCH 7
#define CALENDAR 8
#define WINDOWS_APP 9
#define APPS 10
#define WINDOW_OPEN_STEPS 4
#define WINDOW_RESIZE_STEPS 5
#define WINDOW_CLOSE_STEPS 7
#define SKETCH_W 224
#define SKETCH_H 110
#define CURSOR_W 22
#define CURSOR_H 21
#define CURSOR_BOX_W (CURSOR_W + 1)
#define CURSOR_BOX_H (CURSOR_H + 1)

typedef struct { int x; int y; int w; int h; int opened; int minimized; int order; int maximized; int restore_x; int restore_y; int restore_w; int restore_h; } Window;

static Window windows[APPS];
static const char *app_name[APPS] = {"Files", "Notes", "Console", "System", "Clock", "Calculator", "Appearance", "Sketch", "Calendar", "Windows"};
static int anim_x0[APPS], anim_y0[APPS], anim_w0[APPS], anim_h0[APPS], anim_ticks[APPS], anim_total[APPS], anim_visible[APPS], anim_exit[APPS];
static int quick_open;
static int start_anim;
static int context_anim;
static int quick_anim;
static int notification_open;
static int notification_anim;
static int notification_count;
static int notification_unread;
static char notification_text[5][48];
static char notification_time[5][9];
static int volume_level = 60;
static int volume_muted;
static uint32_t preview_buffer[172 * 92];
static uint32_t *const canvas = (uint32_t *)0x400000;
static uint32_t *const desktop_cache = (uint32_t *)0xD00000;
static int cached_wallpaper = -1;
static int pointer_only;
static int taskbar_dirty = 1;
static uint16_t timer_counter;
static uint32_t timer_accum;
static uint32_t timer_poll;
static uint32_t timer_unchanged;
static int animation_step_allowed = 1;
static int pointer_old_x;
static int pointer_old_y;
static int pointer_saved;
static uint32_t pointer_back[CURSOR_BOX_H][CURSOR_BOX_W];
static uint32_t shadow_pixels;
static uint32_t desktop_regenerations;
static uint32_t full_frames;
static uint32_t pointer_frames;
static uint32_t damage_frames;
static uint32_t presented_pixels;
static int frame_valid;
static int frame_theme = -1, frame_accent = -1, frame_glass = -1, frame_wallpaper = -1;
typedef struct { int x, y, w, h; } DamageRect;
static DamageRect damage[32];
static int damage_count;
static DamageRect last_window_rect[APPS];
static uint8_t last_window_visible[APPS];
static int last_window_order[APPS];
static int last_active = -1;
static int last_start, last_quick, last_context, last_notification, last_hover = -1;
static int last_hover_x, last_hover_running;
static int hover_delay;
static int hover_visible;
static int last_hover_visible;
static int last_context_x, last_context_y, last_snap_side;
static uint32_t round_back[4][18][18];
static int sw;
static int sh;
static int clip_left;
static int clip_top;
static int clip_right;
static int clip_bottom;
static int mouse_x;
static int mouse_y;
static int left_down;
static int drag_app = -1;
static int drag_dx;
static int drag_dy;
static int start_open;
static int next_order;
static int task_hover = -1;
static int desktop_focus = -1;
static int last_desktop_focus = -1;
static int tab_active;
static int tab_apps[APPS];
static int tab_count;
static int tab_selected;
static int last_tab_active;
static int last_tab_selected;
#define PANEL_START 0
#define PANEL_CONTEXT 1
#define PANEL_QUICK 2
#define PANEL_NOTIFICATIONS 3
#define PANEL_RUN 4
#define PANEL_SWITCHER 5
#define PANEL_TOOLTIP 6
#define PANEL_COUNT 7
#define PANEL_STEPS 8
static uint8_t panel_step[PANEL_COUNT];
static uint8_t panel_goal[PANEL_COUNT];
static uint8_t panel_was_visible[PANEL_COUNT];
static int tooltip_render_app = -1;
static int tooltip_render_running;
static int context_open;
static int context_x;
static int context_y;
static int right_down;
static int resize_app = -1;
static int snap_side[APPS];
static int selected_file;
static int sketch_color = 1;
static int sketch_brush = 1;
static uint8_t sketch_pixels[SKETCH_H][SKETCH_W];
static uint8_t sketch_undo[SKETCH_H][SKETCH_W];
static int sketch_has_undo;
static int calendar_shift;
static int calendar_day;
static char menu_query[32];
static int menu_query_len;
static int run_open;
static int run_anim;
static char run_input[40];
static int run_len;
static const uint32_t ink_colors[7] = {0xffffff,0x222d37,0xde9060,0x447e85,0x567cb0,0x73946c,0xb35464};
static int dirty = 1;
static int shift_down;
static int control_down;
static int alt_down;
static uint8_t mouse_packet[3];
static int mouse_index;
static char notes[2048] = "";
static int notes_len;
static char console_input[80];
static int console_len;
static char log_lines[32][72];
static int log_count;
static char cpu_vendor[13];
static char clock_time[9] = "00:00:00";
static char clock_date[11] = "2000-01-01";
static uint8_t last_second = 255;
static char calc_input[32] = "";
static int calc_len;
static char calc_result[32] = "0";
#define CURSOR_W 22
#define CURSOR_H 21
#define CURSOR_BOX_W (CURSOR_W + 1)
#define CURSOR_BOX_H (CURSOR_H + 1)
static const uint8_t cursor_alpha[CURSOR_H][CURSOR_W] = {
    {0,0,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,98,244,239,136,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {4,244,255,255,255,249,160,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,239,255,255,255,255,255,254,184,69,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,136,255,255,255,255,255,255,255,255,209,102,8,0,0,0,0,0,0,0,0,0},
    {0,26,249,255,255,255,255,255,255,255,255,255,231,128,17,0,0,0,0,0,0,0},
    {0,0,160,255,255,255,255,255,255,255,255,255,255,255,242,145,30,0,0,0,0,0},
    {0,0,44,254,255,255,255,255,255,255,255,255,255,255,255,255,253,179,55,0,0,0},
    {0,0,0,184,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,188,43,0},
    {0,0,0,69,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,220,0},
    {0,0,0,0,208,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,5},
    {0,0,0,0,93,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,162,0},
    {0,0,0,0,5,227,255,255,255,255,255,255,255,255,255,255,253,215,163,99,5,0},
    {0,0,0,0,0,116,255,255,255,255,255,255,255,214,114,63,13,0,0,0,0,0},
    {0,0,0,0,0,14,240,255,255,255,255,255,255,119,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,140,255,255,255,255,255,255,68,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,28,250,255,255,255,255,254,18,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,164,255,255,255,255,219,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,46,254,255,255,255,167,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,188,255,255,255,104,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,41,212,250,160,5,0,0,0,0,0,0,0,0,0}
};


static inline uint8_t in8(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}

static int same(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

static int begins(const char *a, const char *b) {
    while (*b) if (*a++ != *b++) return 0;
    return 1;
}

static int size(const char *text) {
    int n = 0;
    while (text[n]) n++;
    return n;
}

static void copy(char *dst, const char *src, int limit) {
    int i = 0;
    while (i < limit - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void append(char *dst, const char *src, int cap) {
    int n = size(dst);
    int i = 0;
    while (src[i] && n < cap - 1) dst[n++] = src[i++];
    dst[n] = 0;
}

static int same_fold(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
        if (ca != cb) return 0;
        i++;
    }
    return !a[i] && !b[i];
}


static void number(char *dst, uint32_t n) {
    char rev[12];
    int i = 0;
    do { rev[i++] = (char)('0' + n % 10); n /= 10; } while (n);
    int j = 0;
    while (i) dst[j++] = rev[--i];
    dst[j] = 0;
}

static uint8_t cmos_read(uint8_t reg) {
    out8(0x70, (uint8_t)(reg | 0x80));
    return in8(0x71);
}

static uint8_t rtc_decimal(uint8_t value, uint8_t control) {
    if (control & 4) return value;
    return (uint8_t)((value & 15) + (value >> 4) * 10);
}

static void rtc_two(char *out, uint8_t n) {
    out[0] = (char)('0' + (n / 10) % 10);
    out[1] = (char)('0' + n % 10);
}

static void refresh_clock(void) {
    if (cmos_read(10) & 0x80) return;
    uint8_t sec = cmos_read(0);
    if (sec == last_second) return;
    last_second = sec;
    uint8_t min = cmos_read(2);
    uint8_t hour = cmos_read(4);
    uint8_t day = cmos_read(7);
    uint8_t month = cmos_read(8);
    uint8_t year = cmos_read(9);
    uint8_t ctrl = cmos_read(11);
    uint8_t pm = hour & 0x80;
    hour = rtc_decimal(hour & 0x7f, ctrl);
    if (!(ctrl & 2)) hour = (uint8_t)((hour % 12) + (pm ? 12 : 0));
    rtc_two(clock_time, hour);
    rtc_two(clock_time + 3, rtc_decimal(min, ctrl));
    rtc_two(clock_time + 6, rtc_decimal(sec, ctrl));
    rtc_two(clock_date + 2, rtc_decimal(year, ctrl));
    rtc_two(clock_date + 5, rtc_decimal(month, ctrl));
    rtc_two(clock_date + 8, rtc_decimal(day, ctrl));
    dirty = 1;
    taskbar_dirty = 1;
}

static void notify(const char *message) {
    if (notification_count == 5) {
        for (int k = 1; k < 5; k++) {
            copy(notification_text[k - 1], notification_text[k], 48);
            copy(notification_time[k - 1], notification_time[k], 9);
        }
        notification_count = 4;
    }
    copy(notification_text[notification_count], message, 48);
    copy(notification_time[notification_count], clock_time, 9);
    notification_count++;
    notification_unread = notification_count;
    dirty = 1;
    taskbar_dirty = 1;
}

static void toggle_desktop(void);
static void open_window(int app);

static void signed_number(char *out, int32_t n) {
    if (n < 0) { out[0] = '-'; number(out + 1, (uint32_t)(-(int64_t)n)); }
    else number(out, (uint32_t)n);
}

static void calculate(void) {
    unsigned i = 0;
    int32_t lhs = 0;
    int32_t rhs = 0;
    while (calc_input[i] >= '0' && calc_input[i] <= '9') {
        lhs = lhs * 10 + (calc_input[i++] - '0');
        if (lhs > 100000) { copy(calc_result, "range", 32); return; }
    }
    char op = calc_input[i++];
    if (op != '+' && op != '-' && op != '*' && op != '/') { copy(calc_result, "syntax", 32); return; }
    if (!calc_input[i]) { copy(calc_result, "syntax", 32); return; }
    while (calc_input[i] >= '0' && calc_input[i] <= '9') {
        rhs = rhs * 10 + (calc_input[i++] - '0');
        if (rhs > 100000) { copy(calc_result, "range", 32); return; }
    }
    if (calc_input[i]) { copy(calc_result, "syntax", 32); return; }
    if (op == '/' && rhs == 0) { copy(calc_result, "division by zero", 32); return; }
    if (op == '*' && lhs > 0 && rhs > 2147483647 / lhs) { copy(calc_result, "range", 32); return; }
    int32_t result = op == '+' ? lhs + rhs : op == '-' ? lhs - rhs : op == '*' ? lhs * rhs : lhs / rhs;
    signed_number(calc_result, result);
}

static void calc_press(char key) {
    if (key == 'C') { calc_len = 0; calc_input[0] = 0; copy(calc_result, "0", 32); }
    else if (key == 'B') { if (calc_len) calc_input[--calc_len] = 0; }
    else if (key == '=') calculate();
    else if (calc_len < 29) {
        if ((key == '+' || key == '-' || key == '*' || key == '/') && calc_len == 0) return;
        calc_input[calc_len++] = key;
        calc_input[calc_len] = 0;
    }
    dirty = 1;
}

static void clip_full(void) {
    clip_left = 0;
    clip_top = 0;
    clip_right = sw;
    clip_bottom = sh;
}

static void pixel(int x, int y, uint32_t color) {
    if (x < clip_left || x >= clip_right || y < clip_top || y >= clip_bottom) return;
    if ((unsigned)x >= (unsigned)sw || (unsigned)y >= (unsigned)sh) return;
    canvas[(size_t)y * (size_t)sw + (size_t)x] = color;
}

static void rect(int x, int y, int w, int h, uint32_t color) {
    int x0 = x > clip_left ? x : clip_left;
    int y0 = y > clip_top ? y : clip_top;
    int x1 = x + w < clip_right ? x + w : clip_right;
    int y1 = y + h < clip_bottom ? y + h : clip_bottom;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > sw) x1 = sw;
    if (y1 > sh) y1 = sh;
    for (int py = y0; py < y1; py++) {
        uint32_t *line = canvas + (size_t)py * (size_t)sw;
        for (int px = x0; px < x1; px++) line[px] = color;
    }
}

static uint32_t blend(uint32_t a, uint32_t b, int weight) {
    int inv = 256 - weight;
    uint32_t r = (((a >> 16) & 255) * inv + ((b >> 16) & 255) * weight) >> 8;
    uint32_t g = (((a >> 8) & 255) * inv + ((b >> 8) & 255) * weight) >> 8;
    uint32_t c = ((a & 255) * inv + (b & 255) * weight) >> 8;
    return (r << 16) | (g << 8) | c;
}

static int ease(int now, int total) {
    if (total <= 0) return 256;
    if (now < 0) now = 0;
    if (now > total) now = total;
    int t = now * 256 / total;
    int inv = 256 - t;
    return 256 - (inv * inv >> 8);
}

static int tween_value(int from, int to, int phase, int total) {
    int e = ease(phase, total);
    return from + ((to - from) * e >> 8);
}

static int panel_visible(int id) {
    return panel_step[id] > 0 || panel_goal[id] > 0;
}

static int panel_shift(int id, int distance) {
    return ((256 - ease(panel_step[id], PANEL_STEPS)) * distance) >> 8;
}

static void panel_sync(void) {
    int desired[PANEL_COUNT] = {
        start_open, context_open, quick_open, notification_open,
        run_open, tab_active,
        hover_visible && task_hover >= 0 && !start_open && !context_open &&
        !quick_open && !notification_open && !run_open && !tab_active
    };
    for (int i = 0; i < PANEL_COUNT; i++) {
        panel_goal[i] = desired[i] ? PANEL_STEPS : 0;
    }
    if (desired[PANEL_TOOLTIP]) {
        tooltip_render_app = task_hover;
        tooltip_render_running = windows[task_hover].opened && !windows[task_hover].minimized;
    }
}

static void panel_advance(void) {
    for (int i = 0; i < PANEL_COUNT; i++) {
        if (panel_step[i] < panel_goal[i]) panel_step[i]++;
        else if (panel_step[i] > panel_goal[i]) {
            panel_step[i]--;
            if (!panel_step[i]) dirty = 1;
        }
    }
}

static void begin_window_animation(int app, int from_x, int from_y, int from_w, int from_h, int steps, int keep_visible) {
    anim_x0[app] = from_x;
    anim_y0[app] = from_y;
    anim_w0[app] = from_w;
    anim_h0[app] = from_h;
    anim_total[app] = steps;
    anim_ticks[app] = steps;
    anim_visible[app] = keep_visible;
    anim_exit[app] = 0;
    dirty = 1;
}

static void animated_window_rect(int app, int *x, int *y, int *w, int *h);

static void close_window(int app, int minimizing) {
    if (!windows[app].opened || windows[app].minimized) return;
    Window *w = &windows[app];
    int x, y, width, height;
    animated_window_rect(app, &x, &y, &width, &height);
    begin_window_animation(app, x, y, width, height, WINDOW_CLOSE_STEPS, 1);
    anim_exit[app] = 1;
    if (minimizing) w->minimized = 1;
    else w->opened = 0;
}

static void animated_window_rect(int app, int *x, int *y, int *w, int *h) {
    *x = windows[app].x;
    *y = windows[app].y;
    *w = windows[app].w;
    *h = windows[app].h;
    if (anim_ticks[app] > 0) {
        int phase = anim_total[app] - anim_ticks[app];
        int to_x = anim_exit[app] ? anim_x0[app] + anim_w0[app] / 7 : windows[app].x;
        int to_y = anim_exit[app] ? anim_y0[app] + 42 : windows[app].y;
        int to_w = anim_exit[app] ? anim_w0[app] * 5 / 7 : windows[app].w;
        int to_h = anim_exit[app] ? anim_h0[app] * 4 / 5 : windows[app].h;
        *x = tween_value(anim_x0[app], to_x, phase, anim_total[app]);
        *y = tween_value(anim_y0[app], to_y, phase, anim_total[app]);
        *w = tween_value(anim_w0[app], to_w, phase, anim_total[app]);
        *h = tween_value(anim_h0[app], to_h, phase, anim_total[app]);
    }
}

static void glass(int x, int y, int w, int h, uint32_t tint, int opacity) {
    int x0 = x > clip_left ? x : clip_left;
    int y0 = y > clip_top ? y : clip_top;
    int x1 = x + w < clip_right ? x + w : clip_right;
    int y1 = y + h < clip_bottom ? y + h : clip_bottom;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > sw) x1 = sw;
    if (y1 > sh) y1 = sh;
    for (int py = y0; py < y1; py++) {
        uint32_t *row = canvas + (size_t)py * sw;
        for (int px = x0; px < x1; px++) row[px] = blend(row[px], tint, opacity);
    }
}

static void blur_region(int x, int y, int w, int h) {
    int left = x > clip_left ? x : clip_left;
    int top = y > clip_top ? y : clip_top;
    int right = x + w < clip_right ? x + w : clip_right;
    int bottom = y + h < clip_bottom ? y + h : clip_bottom;
    if (left < 1) left = 1;
    if (top < 1) top = 1;
    if (right > sw - 1) right = sw - 1;
    if (bottom > sh - 1) bottom = sh - 1;
    if (left >= right || top >= bottom) return;
    for (int yy = top; yy < bottom; yy += 2) {
        int y1 = yy + 1 < bottom ? yy + 1 : yy;
        for (int xx = left; xx < right; xx += 2) {
            int x1 = xx + 1 < right ? xx + 1 : xx;
            uint32_t a = canvas[(size_t)yy * sw + xx];
            uint32_t b = canvas[(size_t)yy * sw + x1];
            uint32_t c = canvas[(size_t)y1 * sw + xx];
            uint32_t d = canvas[(size_t)y1 * sw + x1];
            uint32_t average = ((((a >> 16 & 255) + (b >> 16 & 255) + (c >> 16 & 255) + (d >> 16 & 255)) >> 2) << 16) |
                ((((a >> 8 & 255) + (b >> 8 & 255) + (c >> 8 & 255) + (d >> 8 & 255)) >> 2) << 8) |
                (((a & 255) + (b & 255) + (c & 255) + (d & 255)) >> 2);
            canvas[(size_t)yy * sw + xx] = average;
            canvas[(size_t)yy * sw + x1] = average;
            canvas[(size_t)y1 * sw + xx] = average;
            canvas[(size_t)y1 * sw + x1] = average;
        }
    }
}

static void smooth_pixel(int x, int y, uint32_t color, int coverage) {
    if (coverage <= 0 || x < clip_left || x >= clip_right || y < clip_top || y >= clip_bottom ||
        (unsigned)x >= (unsigned)sw || (unsigned)y >= (unsigned)sh) return;
    uint32_t *dst = canvas + (size_t)y * sw + x;
    *dst = coverage >= 256 ? color : blend(*dst, color, coverage);
}

static void rounded(int x, int y, int w, int h, int radius, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    if (radius < 2 || w < radius * 2 || h < radius * 2) { rect(x, y, w, h, color); return; }
    rect(x + radius, y, w - radius * 2, h, color);
    rect(x, y + radius, radius, h - radius * 2, color);
    rect(x + w - radius, y + radius, radius, h - radius * 2, color);
    for (int row = 0; row < radius; row++) {
        for (int col = 0; col < radius; col++) {
            int coverage = 0;
            for (int a = 1; a <= 3; a += 2)
                for (int b = 1; b <= 3; b += 2) {
                    int dx = radius * 4 - (col * 4 + a);
                    int dy = radius * 4 - (row * 4 + b);
                    if (dx * dx + dy * dy <= radius * radius * 16) coverage += 64;
                }
            if (!coverage) continue;
            smooth_pixel(x + col, y + row, color, coverage);
            smooth_pixel(x + w - 1 - col, y + row, color, coverage);
            smooth_pixel(x + col, y + h - 1 - row, color, coverage);
            smooth_pixel(x + w - 1 - col, y + h - 1 - row, color, coverage);
        }
    }
}

static void round_capture(int x, int y, int w, int h, int r) {
    for (int c = 0; c < 4; c++)
        for (int yy = 0; yy < r; yy++)
            for (int xx = 0; xx < r; xx++) {
                int px = x + ((c & 1) ? w - r : 0) + xx;
                int py = y + ((c & 2) ? h - r : 0) + yy;
                round_back[c][yy][xx] = (unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh ? canvas[(size_t)py * sw + px] : 0;
            }
}

static void round_finish(int x, int y, int w, int h, int r) {
    for (int c = 0; c < 4; c++)
        for (int yy = 0; yy < r; yy++)
            for (int xx = 0; xx < r; xx++) {
                int px = x + ((c & 1) ? w - r : 0) + xx;
                int py = y + ((c & 2) ? h - r : 0) + yy;
                if ((unsigned)px >= (unsigned)sw || (unsigned)py >= (unsigned)sh || px < clip_left || px >= clip_right || py < clip_top || py >= clip_bottom) continue;
                int sx = (c & 1) ? r - 1 - xx : xx;
                int sy = (c & 2) ? r - 1 - yy : yy;
                int coverage = 0;
                for (int a = 1; a <= 3; a += 2)
                    for (int b = 1; b <= 3; b += 2) {
                        int dx = 4 * r - (4 * sx + a);
                        int dy = 4 * r - (4 * sy + b);
                        if (dx * dx + dy * dy <= 16 * r * r) coverage += 64;
                    }
                uint32_t *dst = &canvas[(size_t)py * sw + px];
                if (!coverage) *dst = round_back[c][yy][xx];
                else if (coverage < 256) *dst = blend(round_back[c][yy][xx], *dst, coverage);
            }
}

static void outline(int x, int y, int w, int h, uint32_t color);

static void circle(int cx, int cy, int radius, uint32_t color) {
    if (radius <= 0) return;
    int rr = radius * radius * 16;
    for (int yy = -radius; yy <= radius; yy++) {
        for (int xx = -radius; xx <= radius; xx++) {
            int coverage = 0;
            for (int sy = -1; sy <= 1; sy += 2)
                for (int sx = -1; sx <= 1; sx += 2) {
                    int dx = xx * 4 + sx;
                    int dy = yy * 4 + sy;
                    if (dx * dx + dy * dy <= rr) coverage += 64;
                }
            int px = cx + xx, py = cy + yy;
            if (!coverage || px < clip_left || px >= clip_right || py < clip_top || py >= clip_bottom) continue;
            if ((unsigned)px >= (unsigned)sw || (unsigned)py >= (unsigned)sh) continue;
            uint32_t *dest = canvas + (size_t)py * sw + px;
            *dest = coverage == 256 ? color : blend(*dest, color, coverage);
        }
    }
}

static void glass_frame(int x, int y, int w, int h, uint32_t tint, int strength) {
    glass(x, y, w, h, tint, strength);
    glass(x + 1, y + 1, w - 2, 1, 0xffffff, 120);
    rect(x + 1, y + h - 2, w - 2, 1, 0x7893ac);
    rect(x, y + 1, 1, h - 2, 0x527493);
    rect(x + w - 1, y + 1, 1, h - 2, 0x527493);
}

static void glass_button(int x, int y, int w, int h, uint32_t tint, int strength) {
    glass(x, y, w, h, tint, strength);
    glass(x + 1, y + 1, w - 2, h / 2, 0xffffff, 36);
    outline(x, y, w, h, 0x63819f);
    rect(x + 1, y + 1, w - 2, 1, 0xffffff);
}

static void soft_shadow(int x, int y, int w, int h, int radius, int depth) {
    if (w <= 0 || h <= 0) return;
    for (int d = 16; d >= 1; d--) {
        int rr = radius + d;
        int xx = x - d;
        int yy = y - d + depth;
        int ww = w + d * 2;
        int hh = h + d * 2;
        int strength = 4 + (17 - d) * 2;
        glass(xx + rr, yy, ww - rr * 2, 1, 0x091321, strength);
        glass(xx + rr, yy + hh - 1, ww - rr * 2, 1, 0x091321, strength);
        glass(xx, yy + rr, 1, hh - rr * 2, 0x091321, strength);
        glass(xx + ww - 1, yy + rr, 1, hh - rr * 2, 0x091321, strength);
        shadow_pixels += (uint32_t)((ww - rr * 2 + hh - rr * 2) * 2);
        for (int row = 0; row < rr; row++) {
            int inset = 0;
            int dy = rr - row;
            while (inset < rr && (rr - inset) * (rr - inset) + dy * dy > rr * rr) inset++;
            glass(xx + inset, yy + row, 1, 1, 0x091321, strength);
            glass(xx + ww - inset - 1, yy + row, 1, 1, 0x091321, strength);
            glass(xx + inset, yy + hh - row - 1, 1, 1, 0x091321, strength);
            glass(xx + ww - inset - 1, yy + hh - row - 1, 1, 1, 0x091321, strength);
            shadow_pixels += 4;
        }
    }
}

static void outline(int x, int y, int w, int h, uint32_t color) {
    rect(x, y, w, 1, color);
    rect(x, y + h - 1, w, 1, color);
    rect(x, y, 1, h, color);
    rect(x + w - 1, y, 1, h, color);
}

static void glyph(int x, int y, char c, uint32_t color, int scale) {
    if ((unsigned char)c < 32 || (unsigned char)c > 127) return;
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            int amount = cinmix_font[(int)c - 32][row][col];
            if (!amount) continue;
            if (scale == 1) {
                int px = x + col, py = y + row;
                if (px < clip_left || px >= clip_right || py < clip_top || py >= clip_bottom) continue;
                if ((unsigned)px >= (unsigned)sw || (unsigned)py >= (unsigned)sh) continue;
                uint32_t bg = canvas[(size_t)py * sw + px];
                pixel(px, py, blend(bg, color, amount));
            } else if (amount > 100) rect(x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}

static void text(int x, int y, const char *value, uint32_t color) {
    while (*value && x + 8 <= clip_right) { glyph(x, y, *value++, color, 1); x += 8; }
}

static void big_text(int x, int y, const char *value, uint32_t color) {
    while (*value && x + 16 <= clip_right) { glyph(x, y, *value++, color, 2); x += 16; }
}

static void line(int x1, int y1, int x2, int y2, uint32_t color) {
    int ax = x2 - x1, ay = y2 - y1;
    int steep = (ay < 0 ? -ay : ay) > (ax < 0 ? -ax : ax);
    if (steep) { int t=x1; x1=y1; y1=t; t=x2; x2=y2; y2=t; }
    if (x1 > x2) { int t=x1; x1=x2; x2=t; t=y1; y1=y2; y2=t; }
    int dx=x2-x1;
    if (!dx) { if (steep) pixel(y1,x1,color); else pixel(x1,y1,color); return; }
    int step=((y2-y1)*256)/dx;
    int fy=y1*256;
    for (int x=x1; x<=x2; x++) {
        int y=fy>>8;
        int weight=fy-(y<<8);
        if (steep) { smooth_pixel(y,x,color,256-weight); smooth_pixel(y+1,x,color,weight); }
        else { smooth_pixel(x,y,color,256-weight); smooth_pixel(x,y+1,color,weight); }
        fy+=step;
    }
}

static int active_window(void);

static void draw_mark(int x, int y, uint32_t color) {
    rect(x, y, 9, 4, color);
    rect(x, y, 4, 22, color);
    rect(x + 17, y, 4, 22, color);
    rect(x + 12, y + 18, 9, 4, color);
    rect(x + 8, y + 8, 6, 6, color);
}

static void draw_icon(int x, int y, int id) {
    if (id == FILES) {
        rounded(x + 3, y + 11, 35, 25, 4, 0xe6bc7d);
        rounded(x + 3, y + 6, 16, 10, 3, 0xf4d29b);
        rect(x + 7, y + 17, 27, 3, 0xd7a05c);
        rect(x + 7, y + 32, 27, 1, 0xb17e43);
    } else if (id == NOTES) {
        rounded(x + 7, y + 3, 28, 35, 3, PAPER);
        rect(x + 11, y + 9, 19, 4, BLUE);
        rect(x + 11, y + 19, 19, 2, HAIR);
        rect(x + 11, y + 25, 18, 2, HAIR);
        rect(x + 11, y + 31, 14, 2, HAIR);
    } else if (id == CONSOLE) {
        rounded(x + 2, y + 6, 37, 30, 4, BLACK);
        rect(x + 5, y + 10, 30, 2, 0x638db2);
        line(x + 9, y + 16, x + 17, y + 22, ACTIVE);
        line(x + 17, y + 22, x + 9, y + 28, ACTIVE);
        rect(x + 21, y + 28, 12, 2, WHITE);
    } else if (id == CLOCK) {
        rounded(x + 4, y + 4, 34, 34, 5, LIGHT);
        outline(x + 4, y + 4, 34, 34, BLUE);
        line(x + 21, y + 9, x + 21, y + 22, INK);
        line(x + 21, y + 22, x + 30, y + 27, INK);
    } else if (id == CALC) {
        rounded(x + 8, y + 2, 27, 37, 4, LIGHT);
        rounded(x + 12, y + 7, 19, 9, 2, BLUE);
        for (int k = 0; k < 9; k++) rect(x + 12 + k % 3 * 7, y + 20 + k / 3 * 6, 5, 4, INK2);
    } else if (id == APPEAR) {
        rect(x + 4, y + 4, 16, 16, ACTIVE);
        rect(x + 22, y + 4, 16, 16, BLUE);
        rect(x + 4, y + 22, 16, 16, LIGHT);
        rect(x + 22, y + 22, 16, 16, STEEL);
    } else if (id == CALENDAR) {
        rounded(x + 4, y + 4, 34, 34, 4, WHITE);
        rect(x + 4, y + 9, 34, 9, BLUE);
        text(x + 11, y + 20, "15", INK);
    } else if (id == WINDOWS_APP) {
        rounded(x + 3, y + 7, 36, 29, 4, BLUE);
        rect(x + 7, y + 11, 28, 3, WHITE);
        rect(x + 8, y + 19, 11, 12, 0xb3d4f1);
        rect(x + 21, y + 19, 12, 12, WHITE);
    } else if (id == SKETCH) {
        rounded(x + 4, y + 6, 34, 30, 4, PAPER);
        outline(x + 4, y + 6, 34, 30, STEEL);
        line(x + 10, y + 31, x + 31, y + 12, BLUE);
        rect(x + 25, y + 11, 7, 7, ACTIVE);
    } else {
        draw_mark(x + 10, y + 9, ACTIVE);
    }
}

static const int desk_shortcuts[8] = {FILES, NOTES, CONSOLE, SKETCH, CALENDAR, APPEAR, CLOCK, CALC};

static void desktop_position(int slot, int *x, int *y) {
    if (sw >= 600) { *x = 18 + (slot / 4) * 96; *y = 17 + (slot % 4) * 76; }
    else { *x = 7 + (slot % 4) * 77; *y = 9 + (slot / 4) * 62; }
}

static const int pinned_apps[7] = {FILES, NOTES, CONSOLE, SKETCH, CALENDAR, CALC, APPEAR};

static int pinned_app(int id) {
    for (int i = 0; i < 7; i++) if (pinned_apps[i] == id) return 1;
    return 0;
}

static int taskbar_width(void) { return 46; }
static int taskbar_step(void) { return 54; }

static int taskbar_items(int *items) {
    int maximum = (sw - (sw >= 520 ? 220 : 98)) / taskbar_step();
    if (maximum > APPS) maximum = APPS;
    if (maximum < 1) maximum = 1;
    int count = 0;
    for (int i = 0; i < 7 && count < maximum; i++) items[count++] = pinned_apps[i];
    for (int i = 0; i < APPS && count < maximum; i++) {
        if (windows[i].opened && !pinned_app(i)) items[count++] = i;
    }
    return count;
}

static int taskbar_left(int count) {
    int span = count * taskbar_step() - (taskbar_step() - taskbar_width());
    int x = (sw - span) / 2;
    if (x < 80) x = 80;
    return x;
}

static int taskbar_hit(int x, int y) {
    if (y < sh - 47 || y >= sh - 6) return -1;
    int apps[APPS];
    int count = taskbar_items(apps);
    int start = taskbar_left(count);
    for (int i = 0; i < count; i++) {
        if (x >= start + taskbar_step() * i && x < start + taskbar_step() * i + taskbar_width()) return apps[i];
    }
    return -1;
}

static void draw_task_icon(int x, int y, int id) {
    if (id == FILES) {
        rounded(x + 2, y + 8, 27, 19, 3, 0xeebf72);
        rect(x + 4, y + 5, 13, 8, 0xf5d69b);
        glass(x + 4, y + 13, 23, 3, 0xffffff, 75);
        rect(x + 5, y + 24, 21, 1, 0xb77e39);
    } else if (id == NOTES) {
        rounded(x + 5, y + 2, 21, 27, 2, 0xf8faff);
        rect(x + 8, y + 7, 16, 4, 0x568cc9);
        rect(x + 8, y + 15, 14, 2, 0x92b2cd);
        rect(x + 8, y + 21, 11, 2, 0x92b2cd);
    } else if (id == CONSOLE) {
        rounded(x + 2, y + 4, 28, 23, 4, 0x152a43);
        rect(x + 5, y + 8, 22, 1, 0x739abc);
        line(x + 8, y + 13, x + 14, y + 18, 0xc4edff);
        line(x + 14, y + 18, x + 8, y + 23, 0xc4edff);
        rect(x + 17, y + 22, 7, 2, 0xb2d9ed);
    } else if (id == SKETCH) {
        rounded(x + 3, y + 3, 26, 25, 3, 0xf2f6fc);
        rect(x + 7, y + 8, 18, 15, 0xd8eaf8);
        line(x + 8, y + 21, x + 22, y + 10, 0x246eaf);
        rect(x + 19, y + 9, 5, 5, 0x6e95cf);
    } else if (id == CALENDAR) {
        rounded(x + 3, y + 3, 26, 25, 4, 0xf8fbff);
        rect(x + 3, y + 7, 26, 6, 0x4188bf);
        text(x + 7, y + 14, "15", 0x274365);
    } else if (id == CALC) {
        rounded(x + 6, y + 2, 21, 27, 3, 0xe9f1f9);
        rect(x + 9, y + 6, 15, 6, 0x316ba8);
        for (int k = 0; k < 9; k++) rect(x + 9 + k % 3 * 6, y + 15 + k / 3 * 5, 4, 3, 0x587ca9);
    } else if (id == APPEAR) {
        rounded(x + 3, y + 4, 26, 24, 4, 0xd7e9f6);
        rect(x + 6, y + 7, 10, 9, 0x3e8dce);
        rect(x + 18, y + 7, 8, 9, 0x79b7e4);
        rect(x + 6, y + 18, 10, 7, 0xa2c3db);
        rect(x + 18, y + 18, 8, 7, 0x45698c);
    } else if (id == CLOCK) {
        circle(x + 16, y + 16, 13, 0xe9f6ff);
        circle(x + 16, y + 16, 10, 0x68a5d4);
        circle(x + 16, y + 16, 8, 0xf6fbff);
        line(x + 16, y + 16, x + 16, y + 9, 0x294f80);
        line(x + 16, y + 16, x + 21, y + 19, 0x294f80);
    } else if (id == WINDOWS_APP) {
        rounded(x + 3, y + 5, 26, 22, 3, 0x4c91ce);
        rect(x + 6, y + 8, 20, 3, 0xf4fbff);
        rect(x + 7, y + 15, 7, 8, 0xe4f4ff);
        rect(x + 16, y + 15, 9, 8, 0xb8dafa);
    } else {
        circle(x + 16, y + 16, 12, 0xd8f0ff);
        circle(x + 16, y + 16, 8, 0x4c85b9);
        circle(x + 16, y + 16, 4, 0xeefaff);
    }
}

static void taskbar_mark(int cx, int cy) {
    circle(cx, cy + 1, 23, 0x24405e);
    circle(cx, cy, 21, 0xb6e4fc);
    circle(cx, cy + 1, 18, 0x1771b9);
    circle(cx, cy + 5, 15, 0x09518c);
    glass(cx - 13, cy - 12, 26, 11, 0xffffff, 62);
    draw_mark(cx - 10, cy - 10, 0xffffff);
}

static void regenerate_wallpaper(void) {
    static const uint32_t top_colors[3] = {0x08214e,0x16446a,0x081a32};
    static const uint32_t low_colors[3] = {0x236ca9,0x5ea7be,0x374d8b};
    uint32_t top = top_colors[wallpaper];
    uint32_t bottom = low_colors[wallpaper];
    for (int y = 0; y < sh; y++) {
        uint32_t c = blend(top, bottom, y * 235 / sh);
        for (int x = 0; x < sw; x++) {
            int glow_x = x - (sw * 59 / 100);
            int glow_y = y - (sh * 46 / 100);
            int d = (glow_x * glow_x / (sw > 0 ? sw : 1)) + (glow_y * glow_y / (sh > 0 ? sh : 1));
            int v = 80 - d / 3;
            if (v < 0) v = 0;
            uint32_t sky = wallpaper == 2 ? 0x839be6 : wallpaper == 1 ? 0xbbf5f7 : 0x7cd1f5;
            canvas[(size_t)y * sw + x] = blend(c, sky, v);
        }
    }
    int bottom_y = sh - 49;
    for (int y = sh / 4; y < bottom_y; y++) {
        int u = y - sh / 4;
        int edge = sw * 67 / 100 - u * 4 / 7 + (u * u) / (sh * 2);
        int spread = 95 + u / 3;
        int x0 = edge > 0 ? edge : 0;
        for (int x = x0; x < sw && x < edge + spread; x++) {
            int weight = 99 - (x - edge) * 99 / spread;
            if (weight < 0) weight = 0;
            uint32_t a = canvas[(size_t)y * sw + x];
            pixel(x,y,blend(a,wallpaper == 2 ? 0xaec2ff : 0xc9f2fb,weight));
        }
    }
    for (int y = sh / 3; y < bottom_y; y++) {
        int u = y - sh / 3;
        int edge = sw * 40 / 100 + u * 5 / 8;
        int width = 4 + u * 21 / sh;
        glass(edge, y, width, 1, 0xc6fbff, 100);
        glass(edge + width, y, 6, 1, 0x56ade2, 45);
    }
    for (int y = sh / 2; y < bottom_y; y++) {
        int u = y - sh / 2;
        int edge = sw * 88 / 100 - u * 7 / 9;
        glass(edge, y, 4 + u / 9, 1, 0xeffdff, 84);
    }
}

static void draw_wallpaper(void) {
    if (cached_wallpaper != wallpaper) {
        int old_l = clip_left, old_t = clip_top, old_r = clip_right, old_b = clip_bottom;
        clip_full();
        regenerate_wallpaper();
        for (size_t k = 0; k < (size_t)sw * sh; k++) desktop_cache[k] = canvas[k];
        clip_left = old_l; clip_top = old_t; clip_right = old_r; clip_bottom = old_b;
        cached_wallpaper = wallpaper;
        desktop_regenerations++;
    }
    for (int y = clip_top; y < clip_bottom; y++) {
        size_t offset = (size_t)y * sw + clip_left;
        for (int x = clip_left; x < clip_right; x++) canvas[offset++] = desktop_cache[(size_t)y * sw + x];
    }
}

static void draw_desktop(void) {
    draw_wallpaper();
    for (int k = 0; k < 8; k++) {
        int x,y;
        desktop_position(k, &x, &y);
        if (y + 67 >= sh - 52) continue;
        int id = desk_shortcuts[k];
        draw_icon(x + 20, y + 2, id);
        int tw = size(app_name[id]) * 8;
        text(x + 40 - tw / 2 + 1, y + 52, app_name[id], 0x162746);
        text(x + 40 - tw / 2, y + 51, app_name[id], WHITE);
        if (desktop_focus == k) rounded(x + 21, y + 67, 38, 3, 1, 0xbdefff);
    }
    if (sw >= 600 && sh >= 480) {
        text(sw - 148, sh - 105, "cinmix", 0xd5eeff);
        rect(sw - 148, sh - 84, 69, 1, 0x81ceee);
    }
    glass(0, sh - 52, sw, 52, theme == 2 ? 0x16283d : 0x12344f, 188 + glass_level * 15);
    glass(0, sh - 52, sw, 17, 0xffffff, 48);
    glass(0, sh - 34, sw, 10, 0x1b5285, 45);
    rect(0, sh - 52, sw, 1, 0xd1eefa);
    rect(0, sh - 51, sw, 1, 0x7eadd0);
    taskbar_mark(37, sh - 27);
    int items[APPS];
    int count = taskbar_items(items);
    int left = taskbar_left(count);
    for (int n = 0; n < count; n++) {
        int id = items[n];
        int x = left + n * taskbar_step();
        int active = active_window() == id && !windows[id].minimized;
        int running = windows[id].opened;
        int hovered = task_hover == id;
        if (active || hovered) {
            round_capture(x + 2, sh - 46, taskbar_width() - 4, 39, 8);
            glass(x + 2, sh - 46, taskbar_width() - 4, 39,
                active ? 0x78c4ed : 0xaccfe9, active ? 98 : 67);
            glass(x + 5, sh - 44, taskbar_width() - 10, 12,
                0xffffff, active ? 47 : 35);
            round_finish(x + 2, sh - 46, taskbar_width() - 4, 39, 8);
        }
        draw_task_icon(x + 7, sh - 41, id);
        if (running) {
            if (active) {
                rounded(x + 10, sh - 6, 26, 3, 1, 0xc5f5ff);
            } else {
                rounded(x + 19, sh - 6, 8, 3, 1, 0x79c2f4);
            }
        }
    }
    if (sw >= 700) {
        int bx = sw - 171;
        if (notification_open) rounded(bx - 3, sh - 45, 39, 39, 10, 0x5486a8);
        circle(bx + 17, sh - 29, 8, 0xb6deee);
        rounded(bx + 11, sh - 23, 13, 3, 1, 0xffffff);
        circle(bx + 17, sh - 18, 2, 0xffffff);
        if (notification_unread) {
            circle(bx + 29, sh - 40, 8, 0xe07760);
            char n[4]; number(n, (uint32_t)notification_unread);
            text(bx + 25, sh - 47, n, WHITE);
        }
    }
    if (sw >= 520) {
        glass(sw - 123, sh - 50, 111, 46, 0xffffff, 12);
        text(sw - 111, sh - 42, clock_time, WHITE);
        text(sw - 111, sh - 21, clock_date + 5, 0xd9edfb);
        rect(sw - 11, sh - 51, 1, 50, 0x91bad7);
        glass(sw - 10, sh - 51, 10, 50, 0xbee3fb, 48);
    }
}

static void log_write(const char *value) {
    if (log_count == 32) {
        for (int i = 1; i < 32; i++) copy(log_lines[i - 1], log_lines[i], 72);
        log_count = 31;
    }
    copy(log_lines[log_count++], value, 72);
    dirty = 1;
}

static void print_info(void) {
    log_write("help ver ls cat clear mem cpu date");
    log_write("notes files clock calc sketch theme");
    log_write("calendar windows echo reboot halt");
    log_write("screen snapleft snapright");
    log_write("context resize");
}

static void reboot_machine(void) {
    for (int i = 0; i < 100000; i++) if (!(in8(0x64) & 2)) break;
    out8(0x64, 0xfe);
    for (;;) __asm__ volatile("cli; hlt");
}

static void out16(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" :: "a"(value), "Nd"(port));
}

static void halt_machine(void) {
    out16(0x604, 0x2000);
    out16(0xb004, 0x2000);
    out16(0x4004, 0x3400);
    for (;;) __asm__ volatile("cli; hlt");
}

static void snap_window(int app, int side);

static void execute_command(void) {
    char linebuf[120] = "> ";
    append(linebuf, console_input, 120);
    log_write(linebuf);
    if (!console_len) return;
    if (same(console_input, "help")) print_info();
    else if (same(console_input, "ver")) log_write("Cinmix 0.13.3  |  kernel 0.13.3  |  i386");
    else if (same(console_input, "clear")) log_count = 0;
    else if (same(console_input, "context")) log_write("Right click on desktop");
    else if (same(console_input, "resize")) log_write("Drag window lower-right corner");
    else if (same(console_input, "quick")) log_write("Clock opens Quick Panel");
    else if (same(console_input, "notify")) { notify("Notification from Console"); log_write("Notification delivered"); }
    else if (same(console_input, "ls")) {
        log_write("/ram/readme.txt");
        log_write("/ram/boot.log");
        log_write("/ram/notes.txt");
    } else if (same(console_input, "cat readme.txt")) log_write("Cinmix kernel 0.13.3; El Torito; RAM volume");
    else if (same(console_input, "cat boot.log")) log_write("El Torito -> kernel -> protected mode -> UI");
    else if (same(console_input, "cat notes.txt")) {
        char one[72] = "";
        int i = 0;
        while (notes[i] && notes[i] != '\n' && i < 65) { one[i] = notes[i]; i++; }
        one[i] = 0;
        log_write(one);
    } else if (same(console_input, "mem")) log_write("Physical canvas: 0x00400000");
    else if (same(console_input, "cpu")) log_write(cpu_vendor);
    else if (same(console_input, "date")) { log_write(clock_date); log_write(clock_time); }
    else if (same(console_input, "screen")) {
        char mode[72] = "Display: ";
        char n[12];
        number(n, (uint32_t)sw); append(mode, n, 72);
        append(mode, "x", 72);
        number(n, (uint32_t)sh); append(mode, n, 72);
        append(mode, boot_bpp == 32 ? " 32-bit" : " 8-bit", 72);
        log_write(mode);
    }
    else if (same(console_input, "snapleft") || same(console_input, "snapright")) {
        int a = active_window();
        if (a == CONSOLE) {
            int best = -1;
            for (int i = 0; i < APPS; i++) {
                if (i != CONSOLE && windows[i].opened && !windows[i].minimized && (best < 0 || windows[i].order > windows[best].order)) best = i;
            }
            a = best;
        }
        if (a >= 0) snap_window(a, same(console_input, "snapleft") ? -1 : 1);
        else log_write("No window to snap");
    }
    else if (same(console_input, "clock")) { windows[CLOCK].opened = 1; windows[CLOCK].minimized = 0; windows[CLOCK].order = ++next_order; }
    else if (same(console_input, "calc")) { windows[CALC].opened = 1; windows[CALC].minimized = 0; windows[CALC].order = ++next_order; }
    else if (same(console_input, "sketch")) { windows[SKETCH].opened = 1; windows[SKETCH].minimized = 0; windows[SKETCH].order = ++next_order; }
    else if (same(console_input, "theme")) { windows[APPEAR].opened = 1; windows[APPEAR].minimized = 0; windows[APPEAR].order = ++next_order; }
    else if (same(console_input, "calendar")) { windows[CALENDAR].opened = 1; windows[CALENDAR].minimized = 0; windows[CALENDAR].order = ++next_order; }
    else if (same(console_input, "windows")) { windows[WINDOWS_APP].opened = 1; windows[WINDOWS_APP].minimized = 0; windows[WINDOWS_APP].order = ++next_order; }
    else if (same(console_input, "notes")) { windows[NOTES].opened = 1; windows[NOTES].minimized = 0; windows[NOTES].order = ++next_order; }
    else if (same(console_input, "files")) { windows[FILES].opened = 1; windows[FILES].minimized = 0; windows[FILES].order = ++next_order; }
    else if (same(console_input, "reboot")) reboot_machine();
    else if (same(console_input, "halt")) halt_machine();
    else if (begins(console_input, "echo ")) log_write(console_input + 5);
    else log_write("Unknown command. Try help.");
    console_input[0] = 0;
    console_len = 0;
}

static int active_window(void) {
    int best = -1;
    int order = -1;
    for (int i = 0; i < APPS; i++) {
        if (windows[i].opened && !windows[i].minimized && windows[i].order > order) {
            best = i;
            order = windows[i].order;
        }
    }
    return best;
}

static void snap_window(int app, int side) {
    Window *w = &windows[app];
    if (!w->opened) return;
    int from_x = w->x, from_y = w->y, from_w = w->w, from_h = w->h;
    if (!w->maximized && snap_side[app] == side) {
        w->x = w->restore_x;
        w->y = w->restore_y;
        w->w = w->restore_w;
        w->h = w->restore_h;
        snap_side[app] = 0;
    } else {
        if (!w->maximized && !snap_side[app]) {
            w->restore_x = w->x;
            w->restore_y = w->y;
            w->restore_w = w->w;
            w->restore_h = w->h;
        }
        w->maximized = 0;
        snap_side[app] = side;
        w->x = side < 0 ? 4 : sw / 2 + 2;
        w->y = 4;
        w->w = sw / 2 - 6;
        w->h = sh - 59;
    }
    w->minimized = 0;
    w->order = ++next_order;
    begin_window_animation(app, from_x, from_y, from_w, from_h, WINDOW_RESIZE_STEPS, 0);
    dirty = 1;
}

static void toggle_maximize(int app) {
    Window *w = &windows[app];
    int from_x = w->x, from_y = w->y, from_w = w->w, from_h = w->h;
    if (!w->maximized) {
        if (!snap_side[app]) {
            w->restore_x = w->x; w->restore_y = w->y;
            w->restore_w = w->w; w->restore_h = w->h;
        }
        snap_side[app] = 0;
        w->x = 4; w->y = 4; w->w = sw - 8; w->h = sh - 59;
        w->maximized = 1;
    } else {
        w->x = w->restore_x; w->y = w->restore_y;
        w->w = w->restore_w; w->h = w->restore_h;
        w->maximized = 0;
        snap_side[app] = 0;
    }
    begin_window_animation(app, from_x, from_y, from_w, from_h, WINDOW_RESIZE_STEPS, 0);
    dirty = 1;
}

static int app_from_command(const char *name) {
    if (same_fold(name, "files") || same_fold(name, "explorer")) return FILES;
    if (same_fold(name, "notes") || same_fold(name, "notepad")) return NOTES;
    if (same_fold(name, "console") || same_fold(name, "cmd") || same_fold(name, "terminal")) return CONSOLE;
    if (same_fold(name, "system")) return SYSTEM;
    if (same_fold(name, "clock")) return CLOCK;
    if (same_fold(name, "calculator") || same_fold(name, "calc")) return CALC;
    if (same_fold(name, "appearance") || same_fold(name, "settings")) return APPEAR;
    if (same_fold(name, "sketch") || same_fold(name, "paint")) return SKETCH;
    if (same_fold(name, "calendar")) return CALENDAR;
    if (same_fold(name, "windows")) return WINDOWS_APP;
    return -1;
}

static void run_command(void) {
    if (!run_len) { run_open = 0; dirty = 1; return; }
    if (same_fold(run_input, "desktop")) { run_open = 0; toggle_desktop(); return; }
    if (same_fold(run_input, "reboot")) reboot_machine();
    if (same_fold(run_input, "shutdown") || same_fold(run_input, "halt")) halt_machine();
    int app = app_from_command(run_input);
    if (app >= 0) { run_open = 0; open_window(app); return; }
    notify("Run command not found");
    run_open = 0;
    dirty = 1;
}

static void toggle_desktop(void) {
    if (!desktop_hidden) {
        int found = 0;
        for (int i = 0; i < APPS; i++) {
            desktop_saved[i] = windows[i].opened && !windows[i].minimized;
            if (desktop_saved[i]) { close_window(i, 1); found = 1; }
        }
        desktop_hidden = found;
    } else {
        for (int i = 0; i < APPS; i++) if (desktop_saved[i] && windows[i].opened) open_window(i);
        desktop_hidden = 0;
    }
    start_open = 0;
    context_open = 0;
    quick_open = 0;
    notification_open = 0;
    dirty = 1;
}

static void open_window(int app) {
    desktop_focus = -1;
    desktop_hidden = 0;
    run_open = 0;
    int first = !windows[app].opened || windows[app].minimized;
    int reversing = first && anim_ticks[app] > 0 && anim_exit[app] && anim_visible[app];
    int reverse_x = 0, reverse_y = 0, reverse_w = 0, reverse_h = 0;
    if (reversing) animated_window_rect(app, &reverse_x, &reverse_y, &reverse_w, &reverse_h);
    windows[app].opened = 1;
    windows[app].minimized = 0;
    windows[app].order = ++next_order;
    start_open = 0;
    context_open = 0;
    quick_open = 0;
    notification_open = 0;
    if (first) {
        if (reversing) begin_window_animation(app, reverse_x, reverse_y, reverse_w, reverse_h, WINDOW_OPEN_STEPS, 0);
        else begin_window_animation(app, windows[app].x + windows[app].w / 24, windows[app].y + 12, windows[app].w * 11 / 12, windows[app].h * 11 / 12, WINDOW_OPEN_STEPS, 0);
    }
    dirty = 1;
}

static int date_number(const char *s) { return (s[0]-'0') * 10 + s[1]-'0'; }

static int month_days(int year, int month) {
    static const int lengths[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0)) return 29;
    return lengths[month - 1];
}

static void calendar_month(int *year, int *month) {
    int y = 2000 + date_number(clock_date + 2);
    int m = date_number(clock_date + 5) + calendar_shift;
    while (m < 1) { m += 12; y--; }
    while (m > 12) { m -= 12; y++; }
    *year = y;
    *month = m;
}

static int calendar_weekday(int year, int month) {
    static const int offsets[12] = {0,3,2,5,0,3,5,1,4,6,2,4};
    if (month < 3) year--;
    int d = (year + year/4 - year/100 + year/400 + offsets[month-1] + 1) % 7;
    return (d + 6) % 7;
}

static void snapshot_sketch(void) {
    for (int yy = 0; yy < SKETCH_H; yy++)
        for (int xx = 0; xx < SKETCH_W; xx++) sketch_undo[yy][xx] = sketch_pixels[yy][xx];
    sketch_has_undo = 1;
}

static void undo_sketch(void) {
    if (!sketch_has_undo) return;
    for (int yy = 0; yy < SKETCH_H; yy++)
        for (int xx = 0; xx < SKETCH_W; xx++) sketch_pixels[yy][xx] = sketch_undo[yy][xx];
    sketch_has_undo = 0;
    dirty = 1;
}

static int menu_row_app(int row) {
    int visible = 0;
    for (int i = 0; i < APPS; i++) {
        int match = 1;
        if (menu_query_len) {
            match = 0;
            for (int j = 0; app_name[i][j]; j++) {
                int k = 0;
                while (k < menu_query_len && app_name[i][j+k]) {
                    char a = app_name[i][j+k];
                    char b = menu_query[k];
                    if (a >= 'A' && a <= 'Z') a += 32;
                    if (b >= 'A' && b <= 'Z') b += 32;
                    if (a != b) break;
                    k++;
                }
                if (k == menu_query_len) { match = 1; break; }
            }
        }
        if (match) {
            if (visible == row) return i;
            visible++;
        }
    }
    return -1;
}

static void window_content(int app, const Window *w) {
    int x = w->x + 2;
    int y = w->y + 34;
    int cw = w->w - 4;
    int ch = w->h - 36;
    if (cw < 1 || ch < 1) return;
    int old_l = clip_left, old_t = clip_top, old_r = clip_right, old_b = clip_bottom;
    if (clip_left < x) clip_left = x;
    if (clip_top < y) clip_top = y;
    if (clip_right > x + cw) clip_right = x + cw;
    if (clip_bottom > y + ch) clip_bottom = y + ch;
    if (clip_left >= clip_right || clip_top >= clip_bottom) {
        clip_left = old_l; clip_top = old_t; clip_right = old_r; clip_bottom = old_b;
        return;
    }
    rect(x, y, cw, ch, PAPER);
    if (app == FILES) {
        rect(x, y, 122, ch, LIGHT);
        rect(x + 122, y, 1, ch, HAIR);
        text(x + 12, y + 17, "COMPUTER", INK);
        rect(x + 12, y + 43, 96, 1, HAIR);
        text(x + 16, y + 55, "RAM (R:)", BLUE);
        text(x + 16, y + 82, "3 files", STEEL);
        text(x + 144, y + 14, "R: /", STEEL);
        rect(x + 138, y + 38, cw - 148, 1, HAIR);
        for (int i = 0; i < 3; i++) {
            int yy = y + 52 + i * 37;
            if (selected_file == i) rounded(x + 135, yy - 4, cw - 147, 32, 8, 0xd1dce1);
            rounded(x + 148, yy + 1, 15, 18, 4, WHITE);
            outline(x + 148, yy + 1, 15, 18, STEEL);
            rect(x + 151, yy + 6, 9, 1, HAIR);
            rect(x + 151, yy + 10, 8, 1, HAIR);
            text(x + 174, yy + 2, i == 0 ? "readme.txt" : i == 1 ? "boot.log" : "notes.txt", INK);
        }
        int yy = y + 176;
        rect(x + 137, yy, cw - 150, 1, HAIR);
        text(x + 146, yy + 11, "PREVIEW", STEEL);
        if (selected_file == 0) {
            text(x + 146, yy + 36, "Cinmix 0.13.3", INK);
            text(x + 146, yy + 57, "i386 / El Torito CD", STEEL);
        } else if (selected_file == 1) {
            text(x + 146, yy + 36, "Protected mode", INK);
            text(x + 146, yy + 57, "Framebuffer active", STEEL);
        } else {
            text(x + 146, yy + 36, notes, INK);
            text(x + 146, yy + 57, "Volatile storage", STEEL);
        }
    } else if (app == NOTES) {
        rect(x, y, cw, 27, LIGHT);
        text(x + 12, y + 6, "notes.txt", INK);
        text(x + cw - 90, y + 6, "RAM", STEEL);
        rect(x, y + 27, cw, 1, HAIR);
        int max_columns = (cw - 30) / 8;
        if (max_columns < 1) max_columns = 1;
        int max_lines = (ch - 44) / 19;
        if (max_lines < 1) max_lines = 1;
        int total_line = 0;
        int col = 0;
        for (int i = 0; i < notes_len; i++) {
            if (notes[i] == '\n' || col >= max_columns) { total_line++; col = 0; }
            if (notes[i] != '\n') col++;
        }
        int skip = total_line >= max_lines ? total_line - max_lines + 1 : 0;
        int line_index = 0;
        col = 0;
        for (int i = 0; i <= notes_len; i++) {
            char c = i == notes_len ? 0 : notes[i];
            if (c == '\n' || col >= max_columns) { line_index++; col = 0; }
            if (c && c != '\n') {
                if (line_index >= skip && line_index - skip < max_lines) {
                    glyph(x + 13 + col * 8, y + 36 + (line_index - skip) * 19, c, INK, 1);
                }
                col++;
            }
            if (!c && line_index >= skip && line_index - skip < max_lines) {
                rect(x + 13 + col * 8, y + 36 + (line_index - skip) * 19 + 14, 7, 2, BLUE);
            }
        }
    } else if (app == CONSOLE) {
        rect(x, y, cw, ch, BLACK);
        rect(x, y, cw, 2, ACTIVE);
        int visible = (ch - 45) / 19;
        if (visible < 1) visible = 1;
        int begin = log_count > visible ? log_count - visible : 0;
        int yy = y + 14;
        for (int i = begin; i < log_count; i++) {
            text(x + 12, yy, log_lines[i], 0xcad7de);
            yy += 19;
        }
        text(x + 12, y + ch - 29, ">", ACTIVE);
        text(x + 28, y + ch - 29, console_input, WHITE);
        rect(x + 28 + console_len * 8, y + ch - 13, 7, 2, WHITE);
    } else if (app == SYSTEM) {
        rect(x, y, cw, 4, BLUE);
        big_text(x + 21, y + 22, "CINMIX", INK);
        text(x + 24, y + 62, "RELEASE 0.13.3", STEEL);
        rect(x + 21, y + 91, cw - 42, 1, HAIR);
        text(x + 24, y + 106, "Kernel", STEEL);
        text(x + 166, y + 106, "Cinmix 0.13.3", INK);
        text(x + 24, y + 134, "Target", STEEL);
        text(x + 166, y + 134, "i386 protected", INK);
        text(x + 24, y + 162, "Firmware", STEEL);
        text(x + 166, y + 162, "BIOS / CD-ROM", INK);
        text(x + 24, y + 190, "Processor", STEEL);
        text(x + 166, y + 190, cpu_vendor, INK);
        text(x + 24, y + 218, "Display", STEEL);
        char display[48] = "";
        char tmp[12];
        number(tmp, (uint32_t)sw); append(display, tmp, 48);
        append(display, " x ", 48);
        number(tmp, (uint32_t)sh); append(display, tmp, 48);
        append(display, boot_bpp == 32 ? " / VBE" : " / VGA", 48);
        text(x + 166, y + 218, display, INK);
        text(x + 24, y + 246, "Volume", STEEL);
        text(x + 166, y + 246, "RAM, volatile", INK);
    } else if (app == CLOCK) {
        rect(x, y, cw, 4, BLUE);
        text(x + 22, y + 30, "SYSTEM RTC", STEEL);
        big_text(x + 22, y + 73, clock_time, INK);
        rect(x + 22, y + 127, cw - 44, 1, HAIR);
        big_text(x + 22, y + 156, clock_date, BLUE);
        text(x + 22, y + 230, "CMOS clock", STEEL);
    } else if (app == APPEAR) {
        text(x + 24, y + 19, "COLOR SCHEME", STEEL);
        rect(x + 22, y + 51, cw - 44, 1, HAIR);
        const char *labels[3] = {"Aero", "Silver", "Midnight"};
        const uint32_t samples[3][3] = {
            {0x2167b3,0x96cfee,0xf2f7fc},
            {0x6189b1,0xd4eaf8,0xfafcff},
            {0x122c58,0x578dcc,0xe3eaf7}
        };
        for (int k = 0; k < 3; k++) {
            int yy = y + 67 + k * 66;
            rect(x + 22, yy, cw - 44, 54, theme == k ? LIGHT : PAPER);
            outline(x + 22, yy, cw - 44, 54, theme == k ? BLUE : HAIR);
            rect(x + 32, yy + 10, 60, 34, samples[k][0]);
            rect(x + 48, yy + 19, 24, 13, samples[k][2]);
            rect(x + 48, yy + 19, 24, 3, samples[k][1]);
            text(x + 111, yy + 18, labels[k], INK);
            if (theme == k) text(x + cw - 89, yy + 18, "ACTIVE", BLUE);
        }
        text(x + 24, y + 274, "WALLPAPER", STEEL);
        const char *backgrounds[3] = {"Aurora", "Lagoon", "Twilight"};
        for (int k = 0; k < 3; k++) {
            int px = x + 24 + k * 140;
            rounded(px, y + 298, 129, 43, 5, wallpaper == k ? 0xb8d8f0 : LIGHT);
            outline(px, y + 298, 129, 43, wallpaper == k ? BLUE : HAIR);
            text(px + 14, y + 310, backgrounds[k], INK);
        }
        text(x + 24, y + 358, "GLASS", STEEL);
        for (int k = 0; k < 3; k++) {
            int px = x + 120 + k * 79;
            rounded(px, y + 353, 67, 29, 4, glass_level == k ? 0xb1d9f4 : LIGHT);
            outline(px, y + 353, 67, 29, glass_level == k ? BLUE : HAIR);
            text(px + 15, y + 359, k == 0 ? "Low" : k == 1 ? "Med" : "High", INK);
        }
        text(x + 24, y + 406, "ACCENT", STEEL);
        for (int k = 0; k < 4; k++) {
            int px = x + 120 + k * 62;
            round_capture(px, y + 397, 39, 31, 8);
            rounded(px, y + 397, 39, 31, 8, accent_colors[k]);
            if (accent_index == k) rounded(px + 12, y + 408, 15, 8, 3, WHITE);
            round_finish(px, y + 397, 39, 31, 8);
        }
    } else if (app == SKETCH) {
        text(x + 18, y + 16, "UNTITLED", INK);
        text(x + cw - 93, y + 16, "RAM", STEEL);
        rect(x + 18, y + 44, SKETCH_W * 2, SKETCH_H * 2, WHITE);
        outline(x + 17, y + 43, SKETCH_W * 2 + 2, SKETCH_H * 2 + 2, HAIR);
        for (int row = 0; row < SKETCH_H; row++) {
            for (int col = 0; col < SKETCH_W; col++) {
                uint8_t color = sketch_pixels[row][col];
                if (color) rect(x + 18 + 2 * col, y + 44 + 2 * row, 2, 2, ink_colors[color]);
            }
        }
        rect(x + 18, y + ch - 56, cw - 36, 1, HAIR);
        for (int k = 0; k < 6; k++) {
            int px = x + 18 + 41 * k;
            int py = y + ch - 44;
            rect(px, py, 29, 29, ink_colors[k + 1]);
            if (sketch_color == k + 1) outline(px - 3, py - 3, 35, 35, BLUE);
        }
        rounded(x + cw - 199, y + ch - 43, 85, 29, 6, LIGHT);
        text(x + cw - 177, y + ch - 37, "Undo", sketch_has_undo ? INK : STEEL);
        rounded(x + cw - 102, y + ch - 43, 84, 29, 6, LIGHT);
        text(x + cw - 86, y + ch - 37, "Clear", INK);
    } else if (app == CALENDAR) {
        int year, month;
        calendar_month(&year, &month);
        static const char *months[12] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
        char year_text[12];
        number(year_text, (uint32_t)year);
        big_text(x + 23, y + 21, months[month - 1], INK);
        text(x + 27, y + 66, year_text, STEEL);
        rounded(x + cw - 109, y + 29, 35, 32, 6, LIGHT);
        rounded(x + cw - 65, y + 29, 35, 32, 6, LIGHT);
        text(x + cw - 98, y + 37, "<", INK);
        text(x + cw - 52, y + 37, ">", INK);
        rect(x + 22, y + 94, cw - 44, 1, HAIR);
        static const char *week[7] = {"Mo","Tu","We","Th","Fr","Sa","Su"};
        int cell = (cw - 48) / 7;
        if (cell < 25) cell = 25;
        for (int k = 0; k < 7; k++) text(x + 25 + k * cell, y + 112, week[k], STEEL);
        int start = calendar_weekday(year, month);
        int days = month_days(year, month);
        int today = date_number(clock_date + 8);
        for (int day = 1; day <= days; day++) {
            int k = start + day - 1;
            int cx = x + 21 + (k % 7) * cell;
            int cy = y + 145 + (k / 7) * 34;
            if (cy + 29 > y + ch - 5) continue;
            if (!calendar_shift && day == today) rounded(cx, cy, cell - 5, 29, 5, 0xb4d8f3);
            if (calendar_day == day) outline(cx, cy, cell - 5, 29, BLUE);
            char num[12];
            number(num, (uint32_t)day);
            text(cx + 8, cy + 7, num, INK);
        }
    } else if (app == WINDOWS_APP) {
        text(x + 23, y + 21, "OPEN WINDOWS", STEEL);
        rect(x + 22, y + 51, cw - 44, 1, HAIR);
        int row = 0;
        for (int i = 0; i < APPS; i++) {
            if (!windows[i].opened || i == WINDOWS_APP) continue;
            int cy = y + 63 + 36 * row++;
            if (cy + 33 > y + ch) break;
            rounded(x + 20, cy, cw - 40, 32, 5, LIGHT);
            text(x + 32, cy + 8, app_name[i], INK);
            text(x + 205, cy + 8, windows[i].minimized ? "Minimized" : "Open", STEEL);
            rounded(x + cw - 163, cy + 3, 78, 26, 4, 0xd6e8f7);
            rounded(x + cw - 80, cy + 3, 61, 26, 4, 0xe7e9f0);
            text(x + cw - 154, cy + 7, "Switch", BLUE);
            text(x + cw - 72, cy + 7, "Close", INK);
        }
        if (!row) text(x + 24, y + 73, "No open windows", STEEL);
    } else if (app == CALC) {
        rounded(x + 20, y + 18, 264, 77, 12, LIGHT);
        outline(x + 20, y + 18, 264, 77, HAIR);
        text(x + 30, y + 25, calc_input, STEEL);
        big_text(x + 30, y + 56, calc_result, INK);
        const char *keys = "789/456*123-C0=+";
        for (int k = 0; k < 16; k++) {
            int px = x + 20 + (k % 4) * 66;
            int py = y + 113 + (k / 4) * 47;
            rounded(px, py, 57, 39, 10, k % 4 == 3 ? STEEL : LIGHT);
            outline(px, py, 57, 39, HAIR);
            glyph(px + 24, py + 11, keys[k], k % 4 == 3 ? WHITE : INK, 1);
        }
    }
    clip_left = old_l; clip_top = old_t; clip_right = old_r; clip_bottom = old_b;
}static void draw_window(int i) {
    const Window *w = &windows[i];
    if ((!w->opened || w->minimized) && !anim_visible[i]) return;
    int x, y, ww, hh;
    animated_window_rect(i, &x, &y, &ww, &hh);
    int focused = active_window() == i;
    soft_shadow(x, y, ww, hh, 16, 7);
    round_capture(x + 8, y + 12, ww, hh, 18);
    glass(x + 8, y + 12, ww, hh, 0x06182d, 94);
    round_finish(x + 8, y + 12, ww, hh, 18);
    round_capture(x + 4, y + 6, ww, hh, 18);
    glass(x + 4, y + 6, ww, hh, 0x0b2d50, 47);
    round_finish(x + 4, y + 6, ww, hh, 18);
    round_capture(x, y, ww, hh, 16);
    glass_frame(x, y, ww, hh, focused ? 0xb3dbf7 : 0xcbd5e1, 120 + glass_level * 25);
    glass(x + 2, y + 2, ww - 4, 31, focused ? 0x2e8ac0 : 0x7794a9, focused ? 71 : 52);
    glass(x + 2, y + 2, ww - 4, 12, 0xffffff, 47);
    rect(x + 2, y + 33, ww - 4, 1, 0x6488a5);
    rounded(x + 10, y + 9, 18, 18, 5, 0x2e83b9);
    draw_mark(x + 13, y + 10, WHITE);
    text(x + 39, y + 11, app_name[i], 0x34516d);
    text(x + 38, y + 10, app_name[i], WHITE);
    int by = y + 4;
    round_capture(x + ww - 91, by, 27, 25, 8);
    glass_button(x + ww - 91, by, 27, 25, 0xb5d8ef, 93);
    round_finish(x + ww - 91, by, 27, 25, 8);
    round_capture(x + ww - 61, by, 27, 25, 8);
    glass_button(x + ww - 61, by, 27, 25, 0xa3d3ee, 93);
    round_finish(x + ww - 61, by, 27, 25, 8);
    round_capture(x + ww - 31, by, 27, 25, 8);
    glass_button(x + ww - 31, by, 27, 25, 0xb83f4b, 170);
    round_finish(x + ww - 31, by, 27, 25, 8);
    rounded(x + ww - 83, by + 13, 11, 4, 2, 0x234e6d);
    round_capture(x + ww - 54, by + 8, 12, 10, 3);
    rounded(x + ww - 54, by + 8, 10, 9, 2, blend(WHITE, 0x234e6d, 40));
    round_finish(x + ww - 54, by + 8, 12, 10, 3);
    line(x + ww - 24, by + 8, x + ww - 13, by + 19, WHITE);
    line(x + ww - 13, by + 8, x + ww - 24, by + 19, WHITE);
    Window view = *w;
    view.x = x; view.y = y; view.w = ww; view.h = hh;
    window_content(i, &view);
    if (!w->maximized && !snap_side[i]) {
        line(x + ww - 19, y + hh - 6, x + ww - 6, y + hh - 19, 0x799eb9);
        line(x + ww - 13, y + hh - 6, x + ww - 6, y + hh - 13, 0xa7c4d9);
    }
    round_finish(x, y, ww, hh, 16);
}

static int menu_height(void) { return sh < 540 ? sh - 54 : 442; }
static int menu_top(void) { return sh - 52 - menu_height(); }
static int menu_width(void) { return sw < 510 ? sw - 20 : 490; }
static int menu_left(void) { return 12; }

static void draw_menu(void) {
    if (!panel_visible(PANEL_START)) return;
    int mh = menu_height();
    int my = menu_top();
    int mw = menu_width();
    int mx = menu_left();
    int phase = ease(panel_step[PANEL_START], PANEL_STEPS);
    int offset = ((256 - phase) * 24) >> 8;
    my += offset;
    soft_shadow(mx, my, mw, mh, 18, 8);
    blur_region(mx, my, mw, 73);
    round_capture(mx + 8, my + 9, mw, mh, 18);
    glass(mx + 8, my + 9, mw, mh, 0x071a2d, 102);
    round_finish(mx + 8, my + 9, mw, mh, 18);
    round_capture(mx, my, mw, mh, 18);
    glass_frame(mx, my, mw, mh, 0x96d4fa, 148 + glass_level * 17);
    glass(mx + 2, my + 2, mw - 4, 65, 0x0b5e98, 163);
    glass(mx + 2, my + 2, mw - 4, 20, 0xffffff, 65);
    glass(mx + 6, my + 68, mw - 12, mh - 127, 0xf6fbff, 246);
    rounded(mx + 16, my + 15, 38, 38, 10, 0x1673ae);
    draw_mark(mx + 23, my + 23, WHITE);
    text(mx + 66, my + 14, "Cinmix", WHITE);
    text(mx + 66, my + 35, "Programs", 0xd9efff);
    int list_width = mh < 420 ? mw - 18 : 289;
    int row_h = mh < 420 ? 24 : 29;
    int max_rows = (mh - 135) / row_h;
    if (max_rows > APPS) max_rows = APPS;
    for (int k = 0; k < max_rows; k++) {
        int app = menu_row_app(k);
        if (app < 0) break;
        int py = my + 70 + k * row_h;
        if (menu_query_len && k == 0) rounded(mx + 10, py, list_width - 7, row_h - 2, 8, 0xc3e4f8);
        rounded(mx + 23, py + 5, 17, 17, 5, app == FILES ? 0xc49b62 : 0x3d93bf);
        rect(mx + 27, py + 9, 9, 2, 0xeafaff);
        text(mx + 52, py + (row_h - 16) / 2, app_name[app], INK);
    }
    if (menu_row_app(0) < 0) text(mx + 25, my + 91, "No matches", STEEL);
    if (mh >= 420) {
        glass(mx + 303, my + 69, mw - 310, mh - 126, 0x6da6cf, 85);
        rect(mx + 301, my + 67, 1, mh - 134, 0x7babc9);
        text(mx + 318, my + 84, "PLACES", 0x255777);
        text(mx + 318, my + 123, "Files", INK);
        text(mx + 318, my + 163, "Notes", INK);
        text(mx + 318, my + 203, "System", INK);
        text(mx + 318, my + 243, "Appearance", INK);
        rect(mx + 312, my + 287, mw - 332, 1, 0xb4d1e1);
        text(mx + 318, my + 303, "Restart", INK);
        text(mx + 318, my + 339, "Shut down", INK);
    }
    glass(mx + 2, my + mh - 59, mw - 4, 57, 0x225f93, 137);
    rounded(mx + 15, my + mh - 49, list_width - 13, 35, 10, 0xf8fcff);
    outline(mx + 15, my + mh - 49, list_width - 13, 35, 0x7daed1);
    circle(mx + 28, my + mh - 32, 6, 0x488db8);
    line(mx + 33, my + mh - 27, mx + 37, my + mh - 23, 0x488db8);
    text(mx + 45, my + mh - 40, menu_query_len ? menu_query : "Search programs", menu_query_len ? INK : 0x6387a8);
    round_finish(mx, my, mw, mh, 18);
}

static int context_height(void) { return 170; }
static int context_width(void) { return 208; }

static int quick_width(void) { return 262; }
static int quick_height(void) { return 302; }

static void draw_context(void) {
    if (!panel_visible(PANEL_CONTEXT)) return;
    int x = context_x, y = context_y;
    int w = context_width(), h = context_height();
    y += panel_shift(PANEL_CONTEXT, 12);
    soft_shadow(x, y, w, h, 14, 6);
    blur_region(x, y, w, 56);
    round_capture(x + 5, y + 7, w, h, 15);
    glass(x + 5, y + 7, w, h, 0x05172c, 100);
    round_finish(x + 5, y + 7, w, h, 15);
    round_capture(x, y, w, h, 15);
    glass_frame(x, y, w, h, 0xbeddf1, 188);
    rounded(x + 6, y + 6, w - 12, h - 12, 10, PAPER);
    const char *items[4] = {"Refresh", "Appearance", "Windows", "Show desktop"};
    for (int i = 0; i < 4; i++) {
        int py = y + 13 + i * 38;
        if (mouse_x >= x + 7 && mouse_x < x + w - 7 && mouse_y >= py && mouse_y < py + 34)
            rounded(x + 8, py, w - 16, 33, 10, 0xd7edfb);
        text(x + 23, py + 9, items[i], INK);
    }
    round_finish(x, y, w, h, 15);
}

static void draw_quick(void) {
    if (!panel_visible(PANEL_QUICK)) return;
    int w = quick_width(), h = quick_height();
    int x = sw - w - 16;
    int y = sh - 60 - h;
    y += panel_shift(PANEL_QUICK, 14);
    soft_shadow(x, y, w, h, 16, 8);
    blur_region(x, y, w, 56);
    round_capture(x + 6, y + 8, w, h, 16);
    glass(x + 6, y + 8, w, h, 0x061528, 100);
    round_finish(x + 6, y + 8, w, h, 16);
    round_capture(x, y, w, h, 16);
    glass_frame(x, y, w, h, 0xc7e2f5, 192);
    glass(x + 2, y + 2, w - 4, 46, 0x1e5f93, 145);
    glass(x + 2, y + 2, w - 4, 14, 0xffffff, 56);
    text(x + 18, y + 12, clock_time, WHITE);
    text(x + 18, y + 29, clock_date, 0xd9edfb);
    rounded(x + 16, y + 60, 230, 34, 10, 0xf6fbff);
    rounded(x + 16, y + 102, 110, 34, 10, 0xddecfa);
    rounded(x + 136, y + 102, 110, 34, 10, 0xddecfa);
    rounded(x + 16, y + 144, 230, 28, 10, 0xddecfa);
    text(x + 29, y + 70, "Open Calendar", INK);
    text(x + 29, y + 112, "Appearance", INK);
    text(x + 165, y + 112, "Desktop", INK);
    text(x + 29, y + 151, glass_level == 0 ? "Glass: Light" : glass_level == 1 ? "Glass: Medium" : "Glass: Strong", INK);
    text(x + 16, y + 187, "VOLUME (UI ONLY)", WHITE);
    rounded(x + 16, y + 211, 46, 31, 8, 0xddecfa);
    rounded(x + 72, y + 211, 46, 31, 8, 0xddecfa);
    rounded(x + 129, y + 211, 117, 31, 8, 0xddecfa);
    text(x + 35, y + 217, "-", INK);
    text(x + 91, y + 217, "+", INK);
    text(x + 146, y + 217, volume_muted ? "Unmute" : "Mute", INK);
    char level[8]; number(level, (uint32_t)(volume_muted ? 0 : volume_level));
    text(x + 190, y + 187, level, WHITE);
    text(x + 16, y + 255, "TOOLS", WHITE);
    rounded(x + 16, y + 271, 230, 31, 8, 0xddecfa);
    text(x + 29, y + 278, "Run command", INK);
    round_finish(x, y, w, h, 16);
}

static int notification_width(void) { return 336; }
static int notification_height(void) { return 354; }
static int notification_left(void) { return sw - notification_width() - 14; }
static int notification_top(void) { return sh - 60 - notification_height(); }

static void draw_notifications(void) {
    if (!panel_visible(PANEL_NOTIFICATIONS)) return;
    int w = notification_width(), h = notification_height();
    int x = notification_left();
    int y = notification_top() + panel_shift(PANEL_NOTIFICATIONS, 20);
    soft_shadow(x, y, w, h, 17, 8);
    blur_region(x, y, w, 56);
    round_capture(x, y, w, h, 17);
    glass_frame(x, y, w, h, 0xb6d9ef, 205);
    glass(x + 2, y + 2, w - 4, 52, 0x1e5f93, 154);
    text(x + 18, y + 17, "NOTIFICATIONS", WHITE);
    text(x + w - 93, y + 18, "Clear all", WHITE);
    for (int i = 0; i < notification_count && i < 5; i++) {
        int py = y + 61 + i * 55;
        rounded(x + 12, py, w - 24, 49, 10, 0xecf5fc);
        rounded(x + 13, py + 1, 5, 47, 2, BLUE);
        text(x + 28, py + 9, notification_text[notification_count - 1 - i], INK);
        text(x + 28, py + 28, notification_time[notification_count - 1 - i], STEEL);
        text(x + w - 31, py + 12, "x", STEEL);
    }
    if (!notification_count) {
        text(x + 60, y + 156, "You're all caught up", INK);
        text(x + 60, y + 182, "No new notifications", STEEL);
    }
    round_finish(x, y, w, h, 17);
}

static void draw_snap_preview(void) {
    if (drag_app < 0 || sw < 1000 || (mouse_x > 14 && mouse_x < sw - 15)) return;
    int x = mouse_x <= 14 ? 5 : sw / 2 + 3;
    int y = 5;
    int w = sw / 2 - 9;
    int h = sh - 65;
    round_capture(x, y, w, h, 14);
    glass(x, y, w, h, 0x8fc7f0, 90);
    glass(x + 6, y + 6, w - 12, h - 12, 0xa4dcff, 46);
    glass(x + 10, y + 10, w - 20, h - 20, 0x5ba4df, 19);
    glass(x + 4, y + 3, w - 8, 2, 0xe1f7ff, 120);
    glass(x + 4, y + h - 5, w - 8, 2, 0xe1f7ff, 88);
    round_finish(x, y, w, h, 14);
}

static int run_width(void) { return 366; }
static int run_height(void) { return 146; }
static int run_left(void) { return (sw - run_width()) / 2; }
static int run_top(void) { return sh / 2 - 126; }

static void draw_run(void) {
    if (!panel_visible(PANEL_RUN)) return;
    int w = run_width(), h = run_height();
    int x = run_left();
    int y = run_top() + panel_shift(PANEL_RUN, 15);
    soft_shadow(x, y, w, h, 16, 8);
    round_capture(x, y, w, h, 15);
    glass_frame(x, y, w, h, 0xbedcf2, 198);
    rounded(x + 2, y + 2, w - 4, h - 4, 13, PAPER);
    text(x + 22, y + 18, "Run", INK);
    text(x + 22, y + 40, "Open a program, folder or command.", STEEL);
    rounded(x + 21, y + 67, w - 42, 32, 9, LIGHT);
    outline(x + 21, y + 67, w - 42, 32, 0xc6dceb);
    text(x + 31, y + 76, run_len ? run_input : "files", run_len ? INK : 0x6c87a3);
    rounded(x + w - 103, y + 108, 80, 25, 7, 0xd8ecfa);
    text(x + w - 78, y + 115, "Launch", INK);
    round_finish(x, y, w, h, 15);
}

static void tooltip_size(int hover, int shown, int *w, int *h) {
    *w = shown ? 202 : size(app_name[hover]) * 8 + 30;
    if (*w < 92) *w = 92;
    *h = shown ? 152 : 36;
}

static int tooltip_left(int hover, int w) {
    int items[APPS];
    int count = taskbar_items(items);
    int left = taskbar_left(count);
    int center = mouse_x;
    for (int i = 0; i < count; i++) {
        if (items[i] == hover) { center = left + i * taskbar_step() + taskbar_width() / 2; break; }
    }
    int x = center - w / 2;
    if (x < 8) x = 8;
    if (x + w > sw - 8) x = sw - 8 - w;
    return x;
}

static int tooltip_top(int h) { return sh - 62 - h; }

static void draw_tooltip(void) {
    if (!panel_visible(PANEL_TOOLTIP) || tooltip_render_app < 0) return;
    int hover_app = tooltip_render_app;
    int shown = tooltip_render_running;
    int w, h;
    tooltip_size(hover_app, shown, &w, &h);
    int x = tooltip_left(hover_app, w);
    int y = tooltip_top(h) + panel_shift(PANEL_TOOLTIP, 8);
    if (shown) {
        Window *a = &windows[hover_app];
        for (int py = 0; py < 92; py++) for (int px = 0; px < 172; px++) {
            int sx = a->x + (px * a->w / 172);
            int sy = a->y + (py * a->h / 92);
            preview_buffer[py * 172 + px] = (unsigned)sx < (unsigned)sw && (unsigned)sy < (unsigned)sh ? canvas[(size_t)sy * sw + sx] : BLACK;
        }
    }
    soft_shadow(x, y, w, h, 11, 4);
    round_capture(x, y, w, h, 11);
    glass_frame(x, y, w, h, 0xecf5fc, 230);
    rounded(x + 2, y + 2, w - 4, h - 4, 9, 0xf3f9fe);
    glass(x + 3, y + 3, w - 6, shown ? 30 : 13, 0xffffff, 62);
    if (shown) {
        draw_task_icon(x + 7, y + 4, hover_app);
        text(x + 48, y + 12, app_name[hover_app], INK);
        rounded(x + 11, y + 40, 180, 101, 7, 0xb7cfe2);
        for (int py = 0; py < 92; py++) {
            int yy = y + 44 + py;
            if (yy < clip_top || yy >= clip_bottom || (unsigned)yy >= (unsigned)sh) continue;
            for (int px = 0; px < 172; px++) {
                int xx = x + 15 + px;
                if (xx >= clip_left && xx < clip_right && (unsigned)xx < (unsigned)sw)
                    canvas[(size_t)yy * sw + xx] = preview_buffer[py * 172 + px];
            }
        }
    } else {
        int tw = size(app_name[hover_app]) * 8;
        text(x + (w - tw) / 2, y + 11, app_name[hover_app], INK);
    }
    round_finish(x, y, w, h, 11);
}

static int switcher_left(void) { return (sw - 450) / 2; }
static int switcher_top(void) { return (sh - (tab_count > 5 ? 238 : 154)) / 2; }
static int switcher_height(void) { return tab_count > 5 ? 238 : 154; }

static void draw_switcher(void) {
    if (!panel_visible(PANEL_SWITCHER) || tab_count < 2) return;
    int x = switcher_left(), y = switcher_top() + panel_shift(PANEL_SWITCHER, 15);
    int h = switcher_height();
    soft_shadow(x, y, 450, h, 17, 6);
    round_capture(x, y, 450, h, 16);
    glass_frame(x, y, 450, h, 0x152c47, 230);
    rounded(x + 2, y + 2, 446, h - 4, 14, 0x183451);
    text(x + 21, y + 15, "Switch windows", WHITE);
    text(x + 239, y + 15, "Release Alt to switch", 0xb7d5e9);
    for (int i = 0; i < tab_count; i++) {
        int cx = x + 22 + (i % 5) * 85;
        int cy = y + 46 + (i / 5) * 83;
        if (i == tab_selected) {
            rounded(cx - 6, cy - 5, 73, 72, 10, 0x376d98);
            outline(cx - 6, cy - 5, 73, 72, 0x8bcef4);
        }
        draw_task_icon(cx + 14, cy + 1, tab_apps[i]);
        int len = size(app_name[tab_apps[i]]) * 8;
        if (len > 77) len = 77;
        text(cx + 30 - len / 2, cy + 41, app_name[tab_apps[i]], WHITE);
        if (windows[tab_apps[i]].minimized) rounded(cx + 25, cy + 60, 10, 2, 1, 0x95b2c7);
    }
    round_finish(x, y, 450, h, 16);
}

static void start_or_cycle_switcher(void) {
    if (!tab_active) {
        tab_count = 0;
        int used[APPS] = {0};
        for (int n = 0; n < APPS; n++) {
            int best = -1;
            for (int i = 0; i < APPS; i++) {
                if (!used[i] && windows[i].opened && (best < 0 || windows[i].order > windows[best].order)) best = i;
            }
            if (best < 0) break;
            used[best] = 1;
            tab_apps[tab_count++] = best;
        }
        if (tab_count < 2) return;
        tab_active = 1;
        tab_selected = shift_down ? tab_count - 1 : 1;
        start_open = 0;
        quick_open = 0;
        context_open = 0;
        notification_open = 0;
    } else {
        tab_selected = (tab_selected + (shift_down ? tab_count - 1 : 1)) % tab_count;
    }
    dirty = 1;
}

static void finish_switcher(int commit) {
    if (!tab_active) return;
    int chosen = tab_apps[tab_selected];
    tab_active = 0;
    dirty = 1;
    if (commit && windows[chosen].opened) open_window(chosen);
}

static void draw_pointer(void) {
    pointer_old_x = mouse_x;
    pointer_old_y = mouse_y;
    for (int y = 0; y < CURSOR_BOX_H; y++) {
        for (int x = 0; x < CURSOR_BOX_W; x++) {
            int px = mouse_x + x, py = mouse_y + y;
            pointer_back[y][x] = (unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh ? canvas[(size_t)py * sw + px] : 0;
        }
    }
    for (int y = 0; y < CURSOR_H; y++) {
        for (int x = 0; x < CURSOR_W; x++) {
            int alpha = cursor_alpha[y][x];
            if (!alpha) continue;
            int sx = mouse_x + x + 1, sy = mouse_y + y + 1;
            if ((unsigned)sx < (unsigned)sw && (unsigned)sy < (unsigned)sh) {
                uint32_t *dst = canvas + (size_t)sy * sw + sx;
                *dst = blend(*dst, 0x10223b, alpha * 3 / 7);
            }
        }
    }
    for (int y = 0; y < CURSOR_H; y++) {
        for (int x = 0; x < CURSOR_W; x++) {
            int alpha = cursor_alpha[y][x];
            if (!alpha) continue;
            int px = mouse_x + x, py = mouse_y + y;
            if ((unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh) {
                uint32_t *dst = canvas + (size_t)py * sw + px;
                *dst = blend(*dst, WHITE, alpha);
            }
        }
    }
    pointer_saved = 1;
}

static void present_region(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > sw) w = sw - x;
    if (y + h > sh) h = sh - y;
    if (w <= 0 || h <= 0) return;
    presented_pixels += (uint32_t)(w * h);
    uint8_t *base = (uint8_t *)(uintptr_t)boot_fb;
    for (int yy = y; yy < y + h; yy++) {
        if (boot_bpp == 32) {
            volatile uint32_t *dst = (volatile uint32_t *)(base + (uint32_t)yy * boot_pitch);
            for (int xx = x; xx < x + w; xx++) dst[xx] = canvas[(size_t)yy * sw + xx];
        } else {
            volatile uint8_t *dst = base + (uint32_t)yy * boot_pitch;
            for (int xx = x; xx < x + w; xx++) {
                uint32_t c = canvas[(size_t)yy * sw + xx];
                dst[xx] = (uint8_t)((c & 0xe00000) >> 16 | (c & 0x00e000) >> 11 | (c & 0xc0) >> 6);
            }
        }
    }
}

static void move_pointer_only(void) {
    if (!pointer_saved) { dirty = 1; return; }
    for (int y = 0; y < CURSOR_BOX_H; y++) for (int x = 0; x < CURSOR_BOX_W; x++) {
        int px = pointer_old_x + x, py = pointer_old_y + y;
        if ((unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh)
            canvas[(size_t)py * sw + px] = pointer_back[y][x];
    }
    int old_x = pointer_old_x;
    int old_y = pointer_old_y;
    clip_full();
    draw_pointer();
    present_region(old_x, old_y, CURSOR_BOX_W, CURSOR_BOX_H);
    present_region(mouse_x, mouse_y, CURSOR_BOX_W, CURSOR_BOX_H);
    pointer_frames++;
    pointer_only = 0;
}

static void program_palette(void) {
    out8(0x3c8, 0);
    for (int i = 0; i < 256; i++) {
        out8(0x3c9, (uint8_t)(((i >> 5) & 7) * 9));
        out8(0x3c9, (uint8_t)(((i >> 2) & 7) * 9));
        out8(0x3c9, (uint8_t)((i & 3) * 21));
    }
}

static void present(void) {
    presented_pixels += (uint32_t)(sw * sh);
    uint8_t *base = (uint8_t *)(uintptr_t)boot_fb;
    if (boot_bpp == 32) {
        for (int y = 0; y < sh; y++) {
            volatile uint32_t *dst = (volatile uint32_t *)(base + (uint32_t)y * boot_pitch);
            const uint32_t *src = canvas + (size_t)y * (size_t)sw;
            for (int x = 0; x < sw; x++) dst[x] = src[x];
        }
    } else {
        for (int y = 0; y < sh; y++) {
            volatile uint8_t *dst = base + (uint32_t)y * boot_pitch;
            const uint32_t *src = canvas + (size_t)y * (size_t)sw;
            for (int x = 0; x < sw; x++) {
                uint32_t c = src[x];
                dst[x] = (uint8_t)((c & 0xe00000) >> 16 | (c & 0x00e000) >> 11 | (c & 0xc0) >> 6);
            }
        }
    }
}

static void add_damage(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > sw) w = sw - x;
    if (y + h > sh) h = sh - y;
    if (w <= 0 || h <= 0) return;
    int x2 = x + w, y2 = y + h;
    for (int k = 0; k < damage_count;) {
        DamageRect *d = &damage[k];
        int ux = x < d->x ? x : d->x, uy = y < d->y ? y : d->y;
        int ux2 = x2 > d->x + d->w ? x2 : d->x + d->w;
        int uy2 = y2 > d->y + d->h ? y2 : d->y + d->h;
        int enclosing = (ux2 - ux) * (uy2 - uy);
        int pieces = (x2 - x) * (y2 - y) + d->w * d->h;
        if (x <= d->x + d->w && x2 >= d->x && y <= d->y + d->h && y2 >= d->y && enclosing * 100 <= pieces * 118) {
            if (d->x < x) x = d->x;
            if (d->y < y) y = d->y;
            if (d->x + d->w > x2) x2 = d->x + d->w;
            if (d->y + d->h > y2) y2 = d->y + d->h;
            damage[k] = damage[--damage_count];
            k = 0;
        } else k++;
    }
    if (damage_count >= 32) { damage_count = 1; damage[0] = (DamageRect){0, 0, sw, sh}; return; }
    damage[damage_count++] = (DamageRect){x, y, x2 - x, y2 - y};
}

static void add_window_damage(DamageRect d) {
    add_damage(d.x - 24, d.y - 24, d.w + 48, d.h + 56);
}

static int snap_preview_side(void) {
    if (drag_app < 0 || sw < 1000) return 0;
    if (mouse_x <= 14) return -1;
    if (mouse_x >= sw - 15) return 1;
    return 0;
}

static void add_preview_damage(int side) {
    if (side) add_damage(side < 0 ? 0 : sw / 2 - 2, 0, sw / 2 + 2, sh - 58);
}

static void add_tooltip_damage(int hover, int shown, int visible) {
    if (hover < 0 || hover >= APPS || !visible) return;
    int w, h;
    tooltip_size(hover, shown, &w, &h);
    int left = tooltip_left(hover, w);
    add_damage(left - 19, tooltip_top(h) - 20, w + 38, h + 43);
}

static void draw_scene(void) {
    draw_desktop();
    int placed[APPS] = {0};
    for (int n = 0; n < APPS; n++) {
        int pick = -1;
        for (int i = 0; i < APPS; i++) {
            if (placed[i] || ((!windows[i].opened || windows[i].minimized) && !anim_visible[i])) continue;
            if (pick == -1 || windows[i].order < windows[pick].order) pick = i;
        }
        if (pick < 0) break;
        placed[pick] = 1;
        draw_window(pick);
    }
    draw_snap_preview();
    draw_menu();
    draw_context();
    draw_quick();
    draw_notifications();
    draw_run();
    if (panel_visible(PANEL_SWITCHER)) draw_switcher();
    else draw_tooltip();
}

static void repaint(void) {
    panel_sync();
    int entire = !frame_valid || frame_theme != theme || frame_wallpaper != wallpaper ||
        frame_glass != glass_level || frame_accent != accent_index;
    DamageRect current[APPS];
    int visible[APPS];
    clip_full();
    if (pointer_saved) {
        for (int y = 0; y < CURSOR_BOX_H; y++) for (int x = 0; x < CURSOR_BOX_W; x++) {
            int px = pointer_old_x + x, py = pointer_old_y + y;
            if ((unsigned)px < (unsigned)sw && (unsigned)py < (unsigned)sh)
                canvas[(size_t)py * sw + px] = pointer_back[y][x];
        }
        pointer_saved = 0;
    }
    damage_count = 0;

    int changed_window = 0;
    int order_changed = 0;
    for (int i = 0; i < APPS; i++) {
        visible[i] = (windows[i].opened && !windows[i].minimized) || anim_visible[i];
        current[i] = (DamageRect){0, 0, 0, 0};
        if (visible[i]) {
            animated_window_rect(i, &current[i].x, &current[i].y, &current[i].w, &current[i].h);
        }
        if (!entire) {
            int differs = last_window_visible[i] != visible[i];
            if (visible[i] && last_window_visible[i]) {
                DamageRect d = last_window_rect[i];
                if (d.x != current[i].x || d.y != current[i].y || d.w != current[i].w || d.h != current[i].h) differs = 1;
                if (last_window_order[i] != windows[i].order) order_changed = 1;
            }
            if (differs) {
                changed_window = 1;
                if (last_window_visible[i]) add_window_damage(last_window_rect[i]);
                if (visible[i]) add_window_damage(current[i]);
            }
        }
    }
    if (!entire) {
        if (taskbar_dirty || changed_window || order_changed || last_active != active_window() ||
            last_start != start_open || last_quick != quick_open ||
            last_notification != notification_open || last_context != context_open || last_hover != task_hover || last_hover_visible != hover_visible) {
            add_damage(0, sh - 52, sw, 52);
        }
        if (order_changed) {
            for (int i = 0; i < APPS; i++) {
                if (visible[i]) add_window_damage(current[i]);
                if (last_window_visible[i]) add_window_damage(last_window_rect[i]);
            }
        }
        if (dirty && !changed_window && !order_changed && !start_open && !context_open && !quick_open && !notification_open &&
            !last_start && !last_context && !last_quick && !last_notification && last_hover == task_hover && !taskbar_dirty) {
            int focus = active_window();
            if (focus >= 0 && visible[focus]) add_window_damage(current[focus]);
            if (last_active >= 0 && last_active != focus && last_window_visible[last_active]) add_window_damage(last_window_rect[last_active]);
        }
        if (panel_visible(PANEL_START) || panel_was_visible[PANEL_START] || last_start) add_damage(menu_left() - 24, menu_top() - 26, menu_width() + 48, menu_height() + 55);
        if (panel_visible(PANEL_CONTEXT) || panel_was_visible[PANEL_CONTEXT]) add_damage(context_x - 23, context_y - 25, context_width() + 46, context_height() + 57);
        if (last_context) add_damage(last_context_x - 23, last_context_y - 25, context_width() + 46, context_height() + 57);
        if (panel_visible(PANEL_QUICK) || panel_was_visible[PANEL_QUICK] || last_quick) add_damage(sw - quick_width() - 39, sh - 60 - quick_height() - 24, quick_width() + 62, quick_height() + 55);
        if (panel_visible(PANEL_NOTIFICATIONS) || panel_was_visible[PANEL_NOTIFICATIONS] || last_notification) add_damage(notification_left() - 25, notification_top() - 25, notification_width() + 50, notification_height() + 58);
        if (panel_visible(PANEL_RUN) || panel_was_visible[PANEL_RUN] || run_anim) add_damage(run_left() - 24, run_top() - 24, run_width() + 48, run_height() + 56);
        add_tooltip_damage(last_hover, last_hover_running, last_hover_visible);
        add_tooltip_damage(tooltip_render_app, tooltip_render_running, panel_visible(PANEL_TOOLTIP) || panel_was_visible[PANEL_TOOLTIP]);
        add_preview_damage(last_snap_side);
        add_preview_damage(snap_preview_side());
        if (desktop_focus != last_desktop_focus) {
            if (last_desktop_focus >= 0) {
                int x, y; desktop_position(last_desktop_focus, &x, &y);
                add_damage(x - 2, y - 2, 88, 76);
            }
            if (desktop_focus >= 0) {
                int x, y; desktop_position(desktop_focus, &x, &y);
                add_damage(x - 2, y - 2, 88, 76);
            }
        }
        if (panel_visible(PANEL_SWITCHER) || panel_was_visible[PANEL_SWITCHER] || last_tab_active) {
            add_damage((sw - 450) / 2 - 24, (sh - 238) / 2 - 24, 498, 286);
        }
        add_damage(pointer_old_x, pointer_old_y, CURSOR_BOX_W + 1, CURSOR_BOX_H + 1);
        add_damage(mouse_x, mouse_y, CURSOR_BOX_W + 1, CURSOR_BOX_H + 1);
    }
    if (entire) {
        clip_full();
        draw_scene();
        draw_pointer();
        present();
        full_frames++;
    } else {
        for (int i = 0; i < damage_count; i++) {
            DamageRect d = damage[i];
            clip_left = d.x; clip_top = d.y; clip_right = d.x + d.w; clip_bottom = d.y + d.h;
            draw_scene();
        }
        clip_full();
        draw_pointer();
        for (int i = 0; i < damage_count; i++) {
            DamageRect d = damage[i];
            present_region(d.x, d.y, d.w, d.h);
        }
        damage_frames++;
    }
    for (int i = 0; i < APPS; i++) {
        last_window_rect[i] = current[i];
        last_window_visible[i] = (uint8_t)visible[i];
        last_window_order[i] = windows[i].order;
    }
    for (int i = 0; i < PANEL_COUNT; i++) panel_was_visible[i] = (uint8_t)panel_visible(i);
    last_start = start_open;
    last_quick = quick_open;
    last_notification = notification_open;
    last_context = context_open;
    last_context_x = context_x;
    last_context_y = context_y;
    last_hover = task_hover;
    last_hover_x = mouse_x;
    last_hover_running = task_hover >= 0 && windows[task_hover].opened && !windows[task_hover].minimized;
    last_hover_visible = hover_visible;
    last_active = active_window();
    last_desktop_focus = desktop_focus;
    last_tab_active = tab_active;
    last_tab_selected = tab_selected;
    last_snap_side = snap_preview_side();
    frame_theme = theme;
    frame_accent = accent_index;
    frame_glass = glass_level;
    frame_wallpaper = wallpaper;
    frame_valid = 1;
    pointer_only = 0;
    taskbar_dirty = 0;
    dirty = 0;
    if (animation_step_allowed) {
        panel_advance();
        for (int i = 0; i < APPS; i++) {
            if (anim_ticks[i] > 0) {
                anim_ticks[i]--;
                if (!anim_ticks[i]) {
                    anim_visible[i] = 0;
                    dirty = 1;
                }
            }
        }
        if (start_anim > 0) start_anim--;
        if (context_anim > 0) context_anim--;
        if (quick_anim > 0) quick_anim--;
        if (notification_anim > 0) notification_anim--;
        if (run_anim > 0) run_anim--;
    }
}

static int inside(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

static void sketch_paint(int px, int py) {
    Window *w = &windows[SKETCH];
    int xx = px - (w->x + 20);
    int yy = py - (w->y + 34 + 44);
    if (xx < 0 || yy < 0 || xx >= SKETCH_W * 2 || yy >= SKETCH_H * 2) return;
    xx /= 2;
    yy /= 2;
    for (int dy = -sketch_brush; dy <= sketch_brush; dy++) {
        for (int dx = -sketch_brush; dx <= sketch_brush; dx++) {
            int tx = xx + dx, ty = yy + dy;
            if (tx >= 0 && ty >= 0 && tx < SKETCH_W && ty < SKETCH_H) sketch_pixels[ty][tx] = (uint8_t)sketch_color;
        }
    }
    dirty = 1;
}

static void handle_click(void) {
    if (tab_active) { finish_switcher(0); return; }
    if (desktop_focus >= 0) { desktop_focus = -1; dirty = 1; }
    if (run_open) {
        int rx = run_left(), ry = run_top();
        if (inside(mouse_x, mouse_y, rx, ry, run_width(), run_height())) {
            if (inside(mouse_x, mouse_y, rx + run_width() - 103, ry + 108, 80, 25)) { run_command(); return; }
            dirty = 1;
            return;
        }
        run_open = 0;
        dirty = 1;
        return;
    }
    if (notification_open) {
        int nx = notification_left(), ny = notification_top();
        if (inside(mouse_x, mouse_y, nx, ny, notification_width(), notification_height())) {
            if (inside(mouse_x, mouse_y, nx + notification_width() - 105, ny + 7, 102, 45)) {
                notification_count = 0;
                notification_unread = 0;
            } else {
                for (int i = 0; i < notification_count; i++) {
                    int py = ny + 61 + 55 * i;
                    if (inside(mouse_x, mouse_y, nx + notification_width() - 43, py, 34, 49)) {
                        int at = notification_count - 1 - i;
                        for (int j = at; j < notification_count - 1; j++) {
                            copy(notification_text[j], notification_text[j + 1], 48);
                            copy(notification_time[j], notification_time[j + 1], 9);
                        }
                        notification_count--;
                        if (notification_unread > notification_count) notification_unread = notification_count;
                        break;
                    }
                }
            }
            dirty = 1;
            return;
        }
        notification_open = 0;
        dirty = 1;
        return;
    }
    if (quick_open) {
        int qx = sw - quick_width() - 16;
        int qy = sh - 60 - quick_height();
        if (inside(mouse_x, mouse_y, qx, qy, quick_width(), quick_height())) {
            if (inside(mouse_x, mouse_y, qx + 16, qy + 60, 230, 34)) { open_window(CALENDAR); return; }
            if (inside(mouse_x, mouse_y, qx + 16, qy + 102, 110, 34)) { open_window(APPEAR); return; }
            if (inside(mouse_x, mouse_y, qx + 136, qy + 102, 110, 34)) { toggle_desktop(); return; }
            if (inside(mouse_x, mouse_y, qx + 16, qy + 144, 230, 28)) { glass_level = (glass_level + 1) % 3; }
            if (inside(mouse_x, mouse_y, qx + 16, qy + 211, 46, 31)) { volume_level = volume_level >= 10 ? volume_level - 10 : 0; volume_muted = 0; }
            if (inside(mouse_x, mouse_y, qx + 72, qy + 211, 46, 31)) { volume_level = volume_level <= 90 ? volume_level + 10 : 100; volume_muted = 0; }
            if (inside(mouse_x, mouse_y, qx + 129, qy + 211, 117, 31)) { volume_muted = !volume_muted; }
            if (inside(mouse_x, mouse_y, qx + 16, qy + 271, 230, 31)) { run_open = 1; run_anim = 8; quick_open = 0; dirty = 1; return; }
            dirty = 1;
            return;
        }
        quick_open = 0;
        dirty = 1;
        return;
    }
    if (context_open) {
        if (inside(mouse_x, mouse_y, context_x, context_y, context_width(), context_height())) {
            int selected = (mouse_y - context_y - 13) / 38;
            if (mouse_y >= context_y + 13 && selected >= 0 && selected < 4) {
                context_open = 0;
                if (selected == 1) open_window(APPEAR);
                else if (selected == 2) open_window(WINDOWS_APP);
                else if (selected == 3) toggle_desktop();
            }
        }
        context_open = 0;
        dirty = 1;
        return;
    }
    if (inside(mouse_x, mouse_y, 12, sh - 52, 52, 52)) {
        start_open = !start_open;
        if (start_open) { menu_query_len = 0; menu_query[0] = 0; start_anim = 8; quick_open = 0; }
        dirty = 1;
        return;
    }
    if (start_open) {
        int my = menu_top();
        int mx = menu_left();
        int mw = menu_width();
        int mh = menu_height();
        if (inside(mouse_x, mouse_y, mx, my, mw, mh)) {
            int list_width = mh < 420 ? mw - 18 : 289;
            int row_h = mh < 420 ? 24 : 29;
            int max_rows = (mh - 135) / row_h;
            if (max_rows > APPS) max_rows = APPS;
            for (int k = 0; k < max_rows; k++) {
                int id = menu_row_app(k);
                if (id < 0) break;
                if (inside(mouse_x, mouse_y, mx + 12, my + 70 + k * row_h, list_width - 10, row_h - 2)) {
                    open_window(id);
                    return;
                }
            }
            if (mh >= 420 && inside(mouse_x, mouse_y, mx + 306, my + 69, mw - 316, 300)) {
                int shortcuts[4] = {FILES, NOTES, SYSTEM, APPEAR};
                for (int k = 0; k < 4; k++) {
                    if (inside(mouse_x, mouse_y, mx + 306, my + 116 + 40 * k, mw - 318, 29)) {
                        open_window(shortcuts[k]);
                        return;
                    }
                }
                if (inside(mouse_x, mouse_y, mx + 306, my + 296, mw - 318, 29)) reboot_machine();
                if (inside(mouse_x, mouse_y, mx + 306, my + 331, mw - 318, 29)) halt_machine();
            }
            dirty = 1;
            return;
        }
        start_open = 0;
        dirty = 1;
        return;
    }
    if (mouse_y >= sh - 52) {
        if (mouse_x >= sw - 11) { toggle_desktop(); return; }
        if (mouse_x >= sw - 177 && mouse_x < sw - 125 && sw >= 700) {
            notification_open = !notification_open;
            if (notification_open) { notification_anim = 8; notification_unread = 0; }
            quick_open = 0; start_open = 0; context_open = 0; dirty = 1;
            return;
        }
        if (mouse_x >= sw - 124 && sw >= 520) { quick_open = !quick_open; if (quick_open) quick_anim = 8; notification_open = 0; start_open = 0; context_open = 0; dirty = 1; return; }
        int id = taskbar_hit(mouse_x, mouse_y);
        if (id >= 0) {
            if (windows[id].opened && active_window() == id) { close_window(id, 1); }
            else open_window(id);
            dirty = 1;
            return;
        }
    }
    for (int z = next_order; z >= 0; z--) {
        for (int i = 0; i < APPS; i++) {
            Window *w = &windows[i];
            if (!w->opened || w->minimized || w->order != z) continue;
            int hit_x, hit_y, hit_w, hit_h;
            animated_window_rect(i, &hit_x, &hit_y, &hit_w, &hit_h);
            if (!inside(mouse_x, mouse_y, hit_x, hit_y, hit_w, hit_h) && !inside(mouse_x, mouse_y, w->x, w->y, w->w, w->h)) continue;
            w->order = ++next_order;
            if (inside(mouse_x, mouse_y, hit_x + hit_w - 31, hit_y + 4, 27, 25) || inside(mouse_x, mouse_y, w->x + w->w - 31, w->y + 4, 27, 25)) close_window(i, 0);
            else if (inside(mouse_x, mouse_y, hit_x + hit_w - 61, hit_y + 4, 27, 25) || inside(mouse_x, mouse_y, w->x + w->w - 61, w->y + 4, 27, 25)) toggle_maximize(i);
            else if (inside(mouse_x, mouse_y, hit_x + hit_w - 91, hit_y + 4, 27, 25) || inside(mouse_x, mouse_y, w->x + w->w - 91, w->y + 4, 27, 25)) close_window(i, 1);
            else if (!w->maximized && !snap_side[i] && inside(mouse_x, mouse_y, w->x + w->w - 21, w->y + w->h - 21, 21, 21)) {
                resize_app = i;
                drag_dx = w->x + w->w - mouse_x;
                drag_dy = w->y + w->h - mouse_y;
            }
            else if (inside(mouse_x, mouse_y, w->x, w->y, w->w, 34) && !w->maximized) {
                if (snap_side[i]) {
                    int prev_w = w->restore_w;
                    w->x = mouse_x - prev_w / 2;
                    w->y = mouse_y - 16;
                    w->w = prev_w;
                    w->h = w->restore_h;
                    snap_side[i] = 0;
                }
                drag_app = i;
                drag_dx = mouse_x - w->x;
                drag_dy = mouse_y - w->y;
            } else if (i == CALC) {
                int xx = w->x + 2;
                int yy = w->y + 34;
                const char *keys = "789/456*123-C0=+";
                for (int k = 0; k < 16; k++) {
                    if (inside(mouse_x, mouse_y, xx + 20 + (k % 4) * 66, yy + 113 + (k / 4) * 47, 57, 39)) calc_press(keys[k]);
                }
            } else if (i == FILES) {
                for (int item = 0; item < 3; item++) {
                    if (inside(mouse_x, mouse_y, w->x + 137, w->y + 84 + item * 37, w->w - 148, 35)) selected_file = item;
                }
            } else if (i == APPEAR) {
                int y = w->y + 34;
                for (int k = 0; k < 3; k++) {
                    if (inside(mouse_x, mouse_y, w->x + 24, y + 67 + 66 * k, w->w - 48, 54)) theme = k;
                    if (inside(mouse_x, mouse_y, w->x + 26 + k * 140, y + 298, 129, 43)) wallpaper = k;
                    if (inside(mouse_x, mouse_y, w->x + 122 + k * 79, y + 353, 67, 29)) glass_level = k;
                    if (inside(mouse_x, mouse_y, w->x + 2 + 120 + k * 62, y + 397, 39, 31)) accent_index = k;
                }
            } else if (i == CALENDAR) {
                int xx = w->x + 2;
                int yy = w->y + 34;
                int cw = w->w - 4;
                if (inside(mouse_x, mouse_y, xx + cw - 109, yy + 29, 35, 32) && calendar_shift > -60) { calendar_shift--; calendar_day = 0; }
                if (inside(mouse_x, mouse_y, xx + cw - 65, yy + 29, 35, 32) && calendar_shift < 60) { calendar_shift++; calendar_day = 0; }
                int year, month;
                calendar_month(&year, &month);
                int cell = (cw - 48) / 7;
                if (cell < 25) cell = 25;
                int first = calendar_weekday(year, month);
                for (int day = 1; day <= month_days(year,month); day++) {
                    int k = first + day - 1;
                    int cx = xx + 21 + (k % 7) * cell;
                    int cy = yy + 145 + (k / 7) * 34;
                    if (inside(mouse_x, mouse_y, cx, cy, cell - 5, 29)) calendar_day = day;
                }
            } else if (i == WINDOWS_APP) {
                int xx = w->x + 2;
                int yy = w->y + 34;
                int cw = w->w - 4;
                int row = 0;
                for (int a = 0; a < APPS; a++) {
                    if (!windows[a].opened || a == WINDOWS_APP) continue;
                    int cy = yy + 63 + 36 * row++;
                    if (inside(mouse_x, mouse_y, xx + cw - 163, cy + 3, 78, 26)) { open_window(a); return; }
                    if (inside(mouse_x, mouse_y, xx + cw - 80, cy + 3, 61, 26)) { close_window(a, 0); return; }
                }
            } else if (i == SKETCH) {
                int xx = w->x + 2;
                int yy = w->y + 34;
                if (inside(mouse_x, mouse_y, xx + 18, yy + 44, SKETCH_W * 2, SKETCH_H * 2)) {
                    snapshot_sketch();
                    sketch_paint(mouse_x, mouse_y);
                }
                int bottom = yy + (w->h - 36) - 44;
                for (int k = 0; k < 6; k++) {
                    if (inside(mouse_x, mouse_y, xx + 18 + 41 * k, bottom, 29, 29)) sketch_color = k + 1;
                }
                if (inside(mouse_x, mouse_y, xx + w->w - 4 - 199, bottom + 1, 85, 28)) undo_sketch();
                if (inside(mouse_x, mouse_y, xx + w->w - 4 - 102, bottom + 1, 84, 28)) {
                    snapshot_sketch();
                    for (int row = 0; row < SKETCH_H; row++)
                        for (int col = 0; col < SKETCH_W; col++) sketch_pixels[row][col] = 0;
                }
            }
            dirty = 1;
            return;
        }
    }
    for (int k = 0; k < 8; k++) {
        int x, y;
        desktop_position(k, &x, &y);
        if (y + 72 < sh - 52 && inside(mouse_x, mouse_y, x, y, sw >= 600 ? 87 : 73, sw >= 600 ? 77 : 54)) { open_window(desk_shortcuts[k]); return; }
    }
}

static void handle_right_click(void) {
    if (tab_active) finish_switcher(0);
    desktop_focus = -1;
    run_open = 0;
    context_open = 0;
    start_open = 0;
    quick_open = 0;
    notification_open = 0;
    if (mouse_y >= sh - 53) { dirty = 1; return; }
    for (int i = 0; i < APPS; i++) {
        Window *w = &windows[i];
        if (w->opened && !w->minimized && inside(mouse_x, mouse_y, w->x, w->y, w->w, w->h)) { dirty = 1; return; }
    }
    context_x = mouse_x < sw - context_width() - 8 ? mouse_x : sw - context_width() - 8;
    context_y = mouse_y < sh - context_height() - 60 ? mouse_y : sh - context_height() - 60;
    if (context_x < 6) context_x = 6;
    if (context_y < 6) context_y = 6;
    context_open = 1;
    context_anim = 8;
    dirty = 1;
}

static void handle_mouse_byte(uint8_t b) {
    if (mouse_index == 0 && !(b & 8)) return;
    mouse_packet[mouse_index++] = b;
    if (mouse_index != 3) return;
    mouse_index = 0;
    if (mouse_packet[0] & 0xc0) return;
    int dx = (int8_t)mouse_packet[1];
    int dy = (int8_t)mouse_packet[2];
    int nx = mouse_x + dx;
    int ny = mouse_y - dy;
    if (nx < 0) nx = 0;
    if (ny < 0) ny = 0;
    if (nx >= sw) nx = sw - 1;
    if (ny >= sh) ny = sh - 1;
    if (nx != mouse_x || ny != mouse_y) {
        int hover = taskbar_hit(nx, ny);
        if (hover != task_hover || context_open || drag_app >= 0 || resize_app >= 0 || (left_down && active_window() == SKETCH) || hover_visible) dirty = 1;
        else pointer_only = 1;
        if (hover != task_hover) { hover_delay = 0; hover_visible = 0; }
        task_hover = hover;
        mouse_x = nx;
        mouse_y = ny;
        if (left_down && drag_app < 0 && resize_app < 0 && active_window() == SKETCH) sketch_paint(mouse_x, mouse_y);
        if (left_down && resize_app >= 0) {
            Window *w = &windows[resize_app];
            int width = mouse_x + drag_dx - w->x;
            int height = mouse_y + drag_dy - w->y;
            if (width < 500) width = 500;
            if (height < 390) height = 390;
            if (width > sw - w->x - 5) width = sw - w->x - 5;
            if (height > sh - w->y - 57) height = sh - w->y - 57;
            w->w = width;
            w->h = height;
        }
        if (left_down && drag_app >= 0) {
            Window *w = &windows[drag_app];
            w->x = mouse_x - drag_dx;
            w->y = mouse_y - drag_dy;
            if (w->x < 0) w->x = 0;
            if (w->y < 5) w->y = 5;
            if (w->x > sw - 70) w->x = sw - 70;
            if (w->y > sh - 75) w->y = sh - 75;
        }
    }
    int press = mouse_packet[0] & 1;
    int right = mouse_packet[0] & 2;
    if (right && !right_down) handle_right_click();
    right_down = right;
    if (press && !left_down) handle_click();
    if (!press && left_down && drag_app >= 0 && sw >= 1000) {
        int a = drag_app;
        if (mouse_x <= 12) snap_window(a, -1);
        else if (mouse_x >= sw - 13) snap_window(a, 1);
    }
    if (!press) { drag_app = -1; resize_app = -1; }
    left_down = press;
}

static const char plain_keys[128] = {
    [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0a]='9',[0x0b]='0',
    [0x0c]='-',[0x0d]='=',[0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',[0x15]='y',[0x16]='u',[0x17]='i',
    [0x18]='o',[0x19]='p',[0x1a]='[',[0x1b]=']',[0x1e]='a',[0x1f]='s',[0x20]='d',[0x21]='f',[0x22]='g',[0x23]='h',
    [0x24]='j',[0x25]='k',[0x26]='l',[0x27]=';',[0x28]='\'',[0x29]='`',[0x2b]='\\',[0x2c]='z',[0x2d]='x',
    [0x2e]='c',[0x2f]='v',[0x30]='b',[0x31]='n',[0x32]='m',[0x33]=',',[0x34]='.',[0x35]='/',[0x39]=' '
};

static const char shift_keys[128] = {
    [0x02]='!',[0x03]='@',[0x04]='#',[0x05]='$',[0x06]='%',[0x07]='^',[0x08]='&',[0x09]='*',[0x0a]='(',[0x0b]=')',
    [0x0c]='_',[0x0d]='+',[0x1a]='{',[0x1b]='}',[0x27]=':',[0x28]='"',[0x29]='~',[0x2b]='|',
    [0x33]='<',[0x34]='>',[0x35]='?'
};

static void handle_key(uint8_t code) {
    int released = (code & 0x80) != 0;
    uint8_t key = code & 0x7f;
    if (key == 0x2a || key == 0x36) { shift_down = !released; return; }
    if (key == 0x1d) { control_down = !released; return; }
    if (key == 0x38) {
        alt_down = !released;
        if (released) finish_switcher(1);
        return;
    }
    if (released) return;
    if (key == 0x01) {
        finish_switcher(0);
        desktop_focus = -1;
        run_open = 0;
        start_open = 0; context_open = 0; quick_open = 0; notification_open = 0;
        menu_query_len = 0; menu_query[0] = 0; dirty = 1; return;
    }
    if (alt_down && key == 0x0f) { start_or_cycle_switcher(); return; }
    if (tab_active) return;
    if (control_down && alt_down && key == 0x20) {
        desktop_focus = desktop_focus < 0 ? 0 : -1;
        start_open = 0; context_open = 0; quick_open = 0; notification_open = 0;
        dirty = 1; return;
    }
    if (desktop_focus >= 0) {
        if (key == 0x1c) {
            int app = desk_shortcuts[desktop_focus];
            desktop_focus = -1;
            open_window(app);
            return;
        }
        int next = desktop_focus;
        if (key == 0x48 && next % 4 > 0) next--;
        if (key == 0x50 && next % 4 < 3) next++;
        if (key == 0x4b && next >= 4) next -= 4;
        if (key == 0x4d && next < 4) next += 4;
        if (next != desktop_focus) { desktop_focus = next; dirty = 1; }
        return;
    }
    if (run_open) {
        if (key == 0x0e && run_len > 0) run_input[--run_len] = 0;
        else if (key == 0x1c) { run_command(); return; }
        else if (key == 0x3b || (alt_down && key == 0x13)) { run_open = 0; }
        else if (run_len < 38) {
            char ch = plain_keys[key];
            if (shift_keys[key] && shift_down) ch = shift_keys[key];
            else if (shift_down && ch >= 'a' && ch <= 'z') ch -= 'a' - 'A';
            if (ch && !control_down && !alt_down) { run_input[run_len++] = ch; run_input[run_len] = 0; }
        }
        dirty = 1;
        return;
    }
    if (key == 0x3b) { context_open = 0; quick_open = 0; notification_open = 0; start_open = !start_open; if (start_open) start_anim = 8; menu_query_len = 0; menu_query[0] = 0; dirty = 1; return; }
    if (context_open) { context_open = 0; dirty = 1; return; }
    if (quick_open) { quick_open = 0; dirty = 1; return; }
    if (notification_open) { notification_open = 0; dirty = 1; return; }
    if (control_down && alt_down && key == 0x31) {
        notification_open = !notification_open;
        if (notification_open) { notification_unread = 0; notification_anim = 8; }
        quick_open = 0;
        dirty = 1;
        return;
    }
    if (start_open) {
        if (key == 0x0e && menu_query_len > 0) menu_query[--menu_query_len] = 0;
        else if (key == 0x1c) {
            int id = menu_row_app(0);
            if (id >= 0) open_window(id);
            return;
        } else if (menu_query_len < 30) {
            char ch = plain_keys[key];
            if (shift_keys[key] && shift_down) ch = shift_keys[key];
            if (ch && !control_down && !alt_down) {
                menu_query[menu_query_len++] = ch;
                menu_query[menu_query_len] = 0;
            }
        }
        dirty = 1;
        return;
    }
    if (alt_down && key == 0x20) { toggle_desktop(); return; }
    if (control_down && alt_down && (key == 0x4b || key == 0x4d)) {
        int a = active_window();
        if (a >= 0) snap_window(a, key == 0x4b ? -1 : 1);
        return;
    }
    if (key == 0x57) { int a = active_window(); if (a >= 0) toggle_maximize(a); return; }
    if (alt_down && key == 0x3e) {
        int a = active_window();
        if (a >= 0) close_window(a, 0);
        dirty = 1;
        return;
    }
    if (control_down && key >= 0x02 && key <= 0x08) { open_window(pinned_apps[key - 0x02]); return; }
    if (control_down && key == 0x13) { run_open = !run_open; if (run_open) { run_anim = 8; start_open = 0; context_open = 0; quick_open = 0; notification_open = 0; } dirty = 1; return; }
    if (control_down && key == 0x31) { open_window(NOTES); return; }
    if (control_down && key == 0x2e) { open_window(CONSOLE); return; }
    if (control_down && key == 0x14) { open_window(CLOCK); return; }
    if (control_down && key == 0x1f) { open_window(SKETCH); return; }
    if (control_down && key == 0x20) { open_window(APPEAR); return; }
    if (control_down && key == 0x25) { open_window(CALENDAR); return; }
    if (control_down && key == 0x11) { open_window(WINDOWS_APP); return; }
    int app = active_window();
    if (app != NOTES && app != CONSOLE && app != CALC) return;
    char ch = plain_keys[key];
    if (shift_keys[key] && shift_down) ch = shift_keys[key];
    else if (shift_down && ch >= 'a' && ch <= 'z') ch -= 'a' - 'A';
    if (app == CALC) {
        if (key == 0x0e) calc_press('B');
        else if (key == 0x1c) calc_press('=');
        else if (key == 0x2e && !control_down) calc_press('C');
        else if ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=') calc_press(ch);
        return;
    }
    if (key == 0x0e) {
        if (app == NOTES && notes_len > 0) notes[--notes_len] = 0;
        if (app == CONSOLE && console_len > 0) console_input[--console_len] = 0;
    } else if (key == 0x1c) {
        if (app == NOTES && notes_len < 2046) { notes[notes_len++] = '\n'; notes[notes_len] = 0; }
        else if (app == CONSOLE) execute_command();
    } else if (ch && !control_down) {
        if (app == NOTES && notes_len < 2046) { notes[notes_len++] = ch; notes[notes_len] = 0; }
        if (app == CONSOLE && console_len < 78) { console_input[console_len++] = ch; console_input[console_len] = 0; }
    }
    dirty = 1;
}

static void wait_input_ready(void) {
    for (int i = 0; i < 100000; i++) if (!(in8(0x64) & 2)) return;
}

static void write_controller(uint8_t cmd) {
    wait_input_ready();
    out8(0x64, cmd);
}

static void write_data(uint8_t value) {
    wait_input_ready();
    out8(0x60, value);
}

static int read_data(void) {
    for (int i = 0; i < 1000000; i++) if (in8(0x64) & 1) return in8(0x60);
    return -1;
}

static void write_mouse(uint8_t value) {
    write_controller(0xd4);
    write_data(value);
    read_data();
}

static void init_mouse(void) {
    for (int i = 0; i < 32 && (in8(0x64) & 1); i++) in8(0x60);
    write_controller(0xa8);
    write_controller(0x20);
    int config = read_data();
    if (config >= 0) {
        write_controller(0x60);
        write_data((uint8_t)((config | 3) & ~0x20));
    }
    write_mouse(0xf6);
    write_mouse(0xf4);
}

static int hover_delay_target(void) { return 60; }

static void update_hover_timer(int pulse) {
    int active = task_hover >= 0 && !start_open && !context_open && !quick_open && !notification_open && !run_open && !tab_active;
    if (!active) {
        if (hover_delay || hover_visible) dirty = 1;
        hover_delay = 0;
        hover_visible = 0;
        return;
    }
    if (!pulse) return;
    if (hover_delay < hover_delay_target()) hover_delay++;
    if (!hover_visible && hover_delay >= hover_delay_target()) { hover_visible = 1; dirty = 1; }
}

static int animations_pending(void) {
    for (int i = 0; i < PANEL_COUNT; i++) if (panel_step[i] != panel_goal[i]) return 1;
    if (start_anim || context_anim || quick_anim || notification_anim || run_anim) return 1;
    for (int i = 0; i < APPS; i++) if (anim_ticks[i] > 0) return 1;
    return 0;
}

static uint16_t read_frame_counter(void) {
    out8(0x43, 0x00);
    uint16_t lo = in8(0x40);
    uint16_t hi = in8(0x40);
    return (uint16_t)(lo | (hi << 8));
}

static int advance_frame_counter(uint16_t sample) {
    uint32_t elapsed = timer_counter >= sample ? (uint32_t)(timer_counter - sample) : (uint32_t)(timer_counter + 65535u - sample);
    timer_counter = sample;
    if (elapsed == 0) {
        if (++timer_unchanged >= 2048) {
            timer_unchanged = 0;
            timer_accum = 0;
            return 1;
        }
        return 0;
    }
    timer_unchanged = 0;
    timer_accum += elapsed;
    if (timer_accum < 19886) return 0;
    timer_accum %= 19886;
    return 1;
}

static void init_frame_clock(void) {
    out8(0x43, 0x34);
    out8(0x40, 0xff);
    out8(0x40, 0xff);
    timer_counter = read_frame_counter();
    timer_accum = 0;
    timer_poll = 0;
    timer_unchanged = 0;
}

static int frame_clock_tick(void) {
    if (++timer_poll < 32) return 0;
    timer_poll = 0;
    return advance_frame_counter(read_frame_counter());
}

static void init_cpu(void) {
    uint32_t a, b, c, d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0));
    ((uint32_t *)cpu_vendor)[0] = b;
    ((uint32_t *)cpu_vendor)[1] = d;
    ((uint32_t *)cpu_vendor)[2] = c;
    cpu_vendor[12] = 0;
}

void kmain(void) {
    sw = (int)boot_width;
    sh = (int)boot_height;
    if (sw < 1 || sh < 1 || sw > 1280 || sh > 900) for (;;) __asm__ volatile("cli; hlt");
    init_cpu();
    if (boot_bpp == 8) program_palette();
    mouse_x = sw / 2;
    mouse_y = sh / 2;
    for (int i = 0; i < APPS; i++) {
        windows[i].x = (sw >= 1100 ? (sw - 760) / 2 : sw >= 700 ? 145 : 12) + 14 * (i % 4);
        windows[i].y = (sh >= 700 ? 87 : sh >= 490 ? 65 : 9) + 15 * (i % 4);
        windows[i].w = sw >= 1100 ? 760 : sw >= 700 ? 590 : sw - 25;
        windows[i].h = sh >= 700 ? 485 : sh >= 540 ? 430 : sh - 67;
        windows[i].opened = 0;
    }
    if (sw < 500) {
        for (int i = 0; i < APPS; i++) {
            windows[i].x = 8;
            windows[i].y = 8;
            windows[i].w = sw - 16;
            windows[i].h = sh - 60;
        }
    }
    notes_len = size(notes);
    log_write("Cinmix kernel 0.13.3  [i386]");
    log_write("RAM volume mounted at R:");
    init_mouse();
    init_frame_clock();
    refresh_clock();
    notify("Cinmix 0.13.3 is ready");
    repaint();
    uint32_t cycles = 0;
    for (;;) {
        if ((++cycles & 16383u) == 0) refresh_clock();
        for (int step = 0; step < 64; step++) {
            uint8_t status = in8(0x64);
            if (!(status & 1)) break;
            uint8_t byte = in8(0x60);
            if (status & 0x20) handle_mouse_byte(byte);
            else handle_key(byte);
        }
        int need_pulse = animations_pending() || task_hover >= 0 || hover_visible || run_open;
        int animation_pulse = need_pulse ? frame_clock_tick() : 0;
        if (animation_pulse) update_hover_timer(1);
        else update_hover_timer(0);
        if (dirty || (animation_pulse && animations_pending())) {
            animation_step_allowed = animation_pulse;
            repaint();
            animation_step_allowed = 1;
        } else if (pointer_only) move_pointer_only();
        __asm__ volatile("pause");
    }
}
