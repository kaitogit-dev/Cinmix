#define _GNU_SOURCE
#define CINMIX_STORAGE_TEST
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/storage.c"
#include "../src/objects.c"

static FILE *disk;
static int fail_metadata;
static int disk_writes;
static struct CmMessage received[CM_QUEUE_LIMIT];
static int received_count;
#define check(ok) do { if (!(ok)) { fprintf(stderr, "FAIL objects line %d: %s\n", __LINE__, #ok); exit(1); } } while (0)

int storage_test_read(uint32_t lba, uint8_t *buf) {
    return fseek(disk, (long)lba * 512, SEEK_SET) == 0 && fread(buf, 1, 512, disk) == 512;
}
int storage_test_write(uint32_t lba, const uint8_t *buf) {
    disk_writes++;
    if (fail_metadata && (lba == 1 || lba == 2)) return 0;
    return fseek(disk, (long)lba * 512, SEEK_SET) == 0 && fwrite(buf, 1, 512, disk) == 512 && fflush(disk) == 0;
}
static void observe(int app, const struct CmMessage *ev) {
    check(app == 0 || app == 1);
    check(received_count < CM_QUEUE_LIMIT);
    received[received_count++] = *ev;
}

static void message_tests(void) {
    check(cm_window_open(0) != 0);
    uint32_t old = cm_window_handle(0);
    check(cm_window_open(0) == old);
    check(cm_tasks[0].pid >= 1001 && cm_tasks[0].state == CM_TASK_RUNNING);
    check(cm_post(old, CM_MSG_CREATE, 3, 0));
    check(cm_post(old, CM_MSG_PAINT, 1, 1));
    check(cm_post(old, CM_MSG_PAINT, 2, 2));
    check(cm_post(old, CM_MSG_COMMAND, 7, 8));
    check(cm_queue_size == 3);
    check(cm_pump(1, observe) == 1);
    check(received_count == 1 && received[0].message == CM_MSG_CREATE);
    check(cm_pump(8, observe) == 2);
    check(received_count == 3 && received[1].wparam == 1 && received[2].message == CM_MSG_COMMAND);
    check(cm_tasks[0].events == 3 && cm_tasks[0].paints == 1);
    check(cm_window_open(1) != 0);
    uint32_t second = cm_window_handle(1);
    check(cm_post(old, CM_MSG_MOUSE, 9, 10));
    check(cm_post(second, CM_MSG_KEY, 11, 12));
    cm_window_close(0, 0);
    check(!cm_post(old, CM_MSG_KEY, 2, 3));
    check(cm_pump(8, observe) == 2);
    check(received_count == 4 && received[3].handle == second);
    check(cm_window_open(0) && cm_window_handle(0) != old);
    check(cm_handle_slot(old, CM_OBJECT_WINDOW) == -1);
    cm_window_close(0, 1);
    check(cm_tasks[0].state == CM_TASK_MINIMIZED && cm_window_handle(0));
    cm_window_open(0);
    check(cm_tasks[0].state == CM_TASK_RUNNING);
    cm_window_close(0, 0);
    cm_window_close(1, 0);
    check(cm_handle_count() == 0);
}

int main(int argc, char **argv) {
    check(argc == 3);
    disk = fopen(argv[1], "r+b"); check(disk);
    message_tests();
    char content[4097] = {0};
    if (!strcmp(argv[2], "save")) {
        check(storage_mount());
        check(!cm_file_open(0, "C:/wrong.txt", CM_OPEN_WRITE | CM_OPEN_CREATE));
        check(!cm_file_open(0, "D:/../unsafe", CM_OPEN_WRITE | CM_OPEN_CREATE));
        uint32_t h = cm_file_open(0, "D:/report.txt", CM_OPEN_READ | CM_OPEN_WRITE | CM_OPEN_CREATE);
        check(h && cm_handle_count() == 1);
        check(cm_file_write(h, "Hello", 5) == 5);
        check(cm_file_write(h, " World", 6) == 6);
        check(cm_file_seek(h, 0));
        check(cm_file_read(h, content, 11) == 11 && !memcmp(content, "Hello World", 11));
        check(cm_file_seek(h, 6));
        check(cm_file_write(h, "Cinmix", 6) == 6);
        check(cm_file_write(h, "Z", 4097) == -1);
        check(cm_file_flush(h));
        check(cm_file_close(h));
        check(!cm_file_close(h));
        check(cm_handle_count() == 0);
        check(storage_load("report.txt", content, sizeof(content)) == 12 && !memcmp(content,"Hello Cinmix",12));
        printf("PASS: window handles, message queue, streamed file handles, partial overwrite and committed disk write\n");
    } else if (!strcmp(argv[2], "reload")) {
        check(storage_mount());
        uint32_t h=cm_file_open(1,"D:\\report.txt",CM_OPEN_READ);
        check(h);
        check(cm_file_read(h,content,5)==5 && !memcmp(content,"Hello",5));
        check(cm_file_read(h,content,20)==7 && !memcmp(content," Cinmix",7));
        check(cm_file_close(h));
        h=cm_file_open(0,"D:report.txt",CM_OPEN_WRITE|CM_OPEN_TRUNCATE);
        check(h);
        check(cm_file_write(h,"Interruption",12)==12);
        fail_metadata=1;
        check(!cm_file_close(h));
        check(cm_file_abort(h));
        fail_metadata=0;
        check(storage_mount());
        check(storage_load("report.txt",content,sizeof(content))==12);
        check(!memcmp(content,"Hello Cinmix",12));
        printf("PASS: saved file reopened across processes, aborted commit retains the previous version\n");
    } else if (!strcmp(argv[2], "recover")) {
        check(storage_mount());
        uint32_t h=cm_file_open(1,"report.txt",CM_OPEN_READ);
        check(h);
        check(cm_file_read(h,content,sizeof(content))==12);
        check(!memcmp(content,"Hello Cinmix",12));
        check(cm_file_close(h));
        int old_writes=disk_writes;
        check(!cm_file_open(0,"D:/unsafe/child",CM_OPEN_WRITE|CM_OPEN_CREATE));
        check(disk_writes==old_writes);
        printf("PASS: persistent data and stale-handle protections after interrupted write\n");
    } else check(0);
    check(fclose(disk)==0);
    return 0;
}
