#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include "../src/kernel.c"
uint32_t boot_fb = 0x2000000;
uint16_t boot_pitch = 5120;
uint16_t boot_width = 1280;
uint16_t boot_height = 720;
uint8_t boot_bpp = 32;
static double elapsed(struct timespec a, struct timespec b) { return (b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)/1000000.0; }
int main(void){
  mmap((void*)0x400000, 1280*720*4, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,-1,0);
  mmap((void*)0x900000, 1280*720*4, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,-1,0);
  mmap((void*)0xD00000, 1280*720*4, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,-1,0);
  mmap((void*)0x2000000, 1280*720*4, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,-1,0);
  sw=1280;sh=720;mouse_x=1000;mouse_y=500;
  for(int i=0;i<APPS;i++){windows[i].x=260;windows[i].y=85;windows[i].w=760;windows[i].h=485;}
  struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
  for(int i=0;i<40;i++)repaint();
  clock_gettime(CLOCK_MONOTONIC,&b);printf("desktop_ms_per_frame %.2f\n",elapsed(a,b)/40);
  open_window(FILES);
  for(int i=0;i<8;i++)repaint();
  clock_gettime(CLOCK_MONOTONIC,&a);
  for(int i=0;i<40;i++)repaint();
  clock_gettime(CLOCK_MONOTONIC,&b);printf("window_ms_per_frame %.2f\n",elapsed(a,b)/40);
  return 0;
}
