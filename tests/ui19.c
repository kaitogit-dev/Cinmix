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
#define check(ok) do { if (!(ok)) { fprintf(stderr,"FAIL ui19 line %d: %s\n",__LINE__,#ok); exit(1); } } while (0)
static FILE *disk;
int storage_test_read(uint32_t lba,uint8_t *buf) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fread(buf,1,512,disk)==512;
}
int storage_test_write(uint32_t lba,const uint8_t *buf) {
    return fseek(disk,(long)lba*512,SEEK_SET)==0 && fwrite(buf,1,512,disk)==512 && fflush(disk)==0;
}
static void map_area(uintptr_t a) {
    check(mmap((void*)a,PIXELS*4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=MAP_FAILED);
}
static void frame(void) {
    repaint();
    cm_pump(CM_QUEUE_LIMIT,cm_ui_dispatch);
    check(memcmp(canvas,(void*)(uintptr_t)boot_fb,PIXELS*4)==0);
}
static void frames(int n) { for(int i=0;i<n;i++) frame(); }
static void screenshot(const char *path) {
    FILE *f=fopen(path,"wb"); check(f);
    fprintf(f,"P6\n1920 1080\n255\n");
    for(size_t i=0;i<PIXELS;i++) { uint32_t c=canvas[i]; fputc((c>>16)&255,f); fputc((c>>8)&255,f); fputc(c&255,f); }
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
    sw=1920;sh=1080;mouse_x=1530;mouse_y=860;
    for(int i=0;i<APPS;i++) {windows[i].w=1050;windows[i].h=700;windows[i].x=(sw-1050)/2+14*(i%4);windows[i].y=(sh-700)/2-12+15*(i%4);}
    check(storage_mount());
    check(cm_handle_count()==0);
    open_window(NOTES);
    uint32_t first=cm_window_handle(NOTES);
    check(first && cm_tasks[NOTES].pid && cm_tasks[NOTES].state==CM_TASK_RUNNING);
    frames(WINDOW_OPEN_STEPS+1);
    check(cm_tasks[NOTES].paints>0);
    handle_key(0x23);handle_key(0x12);handle_key(0x26);handle_key(0x26);handle_key(0x18);
    check(!strcmp(notes,"hello") && notes_dirty);
    handle_key(0x1d);handle_key(0x1f);handle_key(0x9d);
    check(!notes_dirty && storage_slot("notes.txt")>=0);
    open_window(FILES);frames(WINDOW_OPEN_STEPS+2);
    Window *w=&windows[FILES];
    click(w->x+2+50,w->y+34+27);
    check(file_created==1 && storage_slot("New document.txt")>=0);
    int x=w->x+2,y=w->y+34;
    file_location=6;file_sort=0;file_view=0;
    click(files_list_left(x,w->w-4)+38,files_row_top(y)+FILE_COUNT*files_row_height()+11);
    check(selected_file==FILE_COUNT);
    handle_key(0x1c);
    check(active_window()==NOTES);
    check(cm_window_handle(NOTES)==first);
    handle_key(0x17);handle_key(0x18);
    check(!strcmp(notes,"io"));
    close_window(NOTES,0);
    check(!cm_window_handle(NOTES) && cm_tasks[NOTES].state==CM_TASK_STOPPED);
    check(!cm_post(first,CM_MSG_KEY,1,2));
    frames(WINDOW_CLOSE_STEPS+1);
    notes[0]=0;notes_len=0;
    open_window(NOTES);
    check(cm_window_handle(NOTES)!=first);
    check(cm_tasks[NOTES].state==CM_TASK_RUNNING);
    notes_select("notes.txt");check(!strcmp(notes,"hello"));
    notes_select("New document.txt");check(!strcmp(notes,"io"));
    frames(WINDOW_OPEN_STEPS+1);
    open_window(CONSOLE);
    frames(WINDOW_OPEN_STEPS+1);
    copy(console_input,"tasklist",80);console_len=8;
    execute_command();check(log_count>1);
    copy(console_input,"handles",80);console_len=7;
    execute_command();
    int found=0;
    for(int k=0;k<log_count;k++) if(begins(log_lines[k],"Kernel handles in use")) found=1;
    check(found);
    open_window(SYSTEM);frames(WINDOW_OPEN_STEPS+1);
    check(cm_handle_count()>=4);
    screenshot("build/kernel-internals-19.ppm");
    storage_ready=0;notes[0]=0;notes_len=0;file_created=0;
    check(storage_mount());files_restore();
    notes_select("New document.txt");check(!strcmp(notes,"io"));
    printf("PASS: 1080p message-driven input, task IDs, generational handles, file stream APIs, Explorer+Notes disk persistence, Console diagnostics and framebuffer consistency\n");
    check(fclose(disk)==0);
    return 0;
}
