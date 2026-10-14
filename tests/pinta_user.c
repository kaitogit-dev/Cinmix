#define _GNU_SOURCE
#define CINMIX_HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include "../src/proc.c"
#include "../src/pinta.c"
#define check(c) do { if (!(c)) { fprintf(stderr,"FAIL pinta_user line %d: %s\n",__LINE__,#c);exit(1); } } while(0)
int main(void) {
    check(mmap((void *)0x03000000u, 0x00800000u, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0) != MAP_FAILED);
    check(cm_proc_initialize());
    check(cm_user_tasks[0].state == CM_THREAD_READY && cm_user_tasks[1].state == CM_THREAD_READY);
    check(cm_task_page(0, CM_PAINT_BUFFER, 1) == cm_user_tasks[0].physical_base + 0x10000u);
    check(cm_task_page(1, CM_PAINT_BUFFER, 1) == cm_user_tasks[1].physical_base + 0x10000u);
    check(cm_task_page(0, CM_PAINT_BUFFER, 1) != cm_task_page(1, CM_PAINT_BUFFER, 1));
    check(cm_task_page(0, 0x400000, 1) == 0);
    check(!cm_paint_submit(-1, 40, 4, 1));
    check(!cm_paint_submit(34, 110, 4, 1));
    check(!cm_paint_submit(34, 40, 4, 13));
    check(cm_paint_submit(20, 10, 3, 2));
    check(cm_paint_pending());
    check(!cm_paint_submit(20, 10, 3, 2));
    uint8_t first[CM_PAINT_WIDTH*CM_PAINT_HEIGHT] = {0};
    check(!cm_paint_collect(first, sizeof(first)));
    CmPaintMailbox *box=(CmPaintMailbox *)(uintptr_t)(cm_user_tasks[0].physical_base+0x2100u);
    uint8_t *buffer=(uint8_t *)(uintptr_t)(cm_user_tasks[0].physical_base+0x10000u);
    pinta_stamp(buffer,CM_PAINT_WIDTH,CM_PAINT_HEIGHT,box->x,box->y,box->color,box->radius);
    box->command=0;box->completed++;
    check(cm_paint_collect(first,sizeof(first)));
    check(!cm_paint_pending() && box->completed==0);
    check(first[10*CM_PAINT_WIDTH+20]==3);
    check(first[0]==0 && buffer[0]==0);
    check(!cm_paint_collect(first,sizeof(first)));
    check(cm_paint_submit(20,10,5,0));
    box->command=0;box->completed=1;
    buffer[10*CM_PAINT_WIDTH+20]=5;
    check(cm_paint_collect(first,sizeof(first)) && first[10*CM_PAINT_WIDTH+20]==5);
    uint8_t second[CM_PAINT_WIDTH * CM_PAINT_HEIGHT]={0};
    pinta_line(second,CM_PAINT_WIDTH,CM_PAINT_HEIGHT,2,2,10,10,7,0);
    check(second[2*CM_PAINT_WIDTH+2]==7 && second[10*CM_PAINT_WIDTH+10]==7);
    pinta_rectangle(second,CM_PAINT_WIDTH,CM_PAINT_HEIGHT,20,20,35,25,2,0);
    check(second[20*CM_PAINT_WIDTH+20]==2 && second[25*CM_PAINT_WIDTH+35]==2);
    uint8_t image[9*9]={0};
    uint32_t scratch[9*9];
    pinta_rectangle(image,9,9,1,1,7,7,4,0);
    check(pinta_fill(image,9,9,4,4,3,scratch,81));
    check(image[4*9+4]==3 && image[0]==0 && image[1*9+1]==4);
    cm_user_tasks[0].state = CM_THREAD_FAULTED;
    check(!cm_paint_submit(20,10,2,1));
    check(!cm_paint_collect(first,sizeof(first)));
    puts("PASS: ring3 paint mailbox, isolated writable mappings, bounded input, paint-buffer results, stale/faulted worker rejection, pencil/line/rect/fill raster geometry");
    return 0;
}
