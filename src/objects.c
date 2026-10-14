#include <stdint.h>
#include <stddef.h>

#define CM_OBJECT_LIMIT 32
#define CM_APP_LIMIT 10
#define CM_QUEUE_LIMIT 96
#define CM_OPEN_READ 1u
#define CM_OPEN_WRITE 2u
#define CM_OPEN_CREATE 4u
#define CM_OPEN_TRUNCATE 8u
#define CM_OBJECT_WINDOW 1
#define CM_OBJECT_FILE 2
#define CM_MSG_CREATE 1
#define CM_MSG_ACTIVATE 2
#define CM_MSG_CLOSE 3
#define CM_MSG_MINIMIZE 4
#define CM_MSG_PAINT 5
#define CM_MSG_KEY 6
#define CM_MSG_CHAR 7
#define CM_MSG_COMMAND 8
#define CM_MSG_MOUSE 9
#define CM_MSG_TIMER 10

#define CM_TASK_STOPPED 0
#define CM_TASK_RUNNING 1
#define CM_TASK_MINIMIZED 2

struct CmObject { uint16_t gen; uint8_t type; uint8_t owner; };
struct CmFile { uint32_t offset, length, flags; uint8_t changed; char name[20]; uint8_t content[4097]; };
struct CmTask { uint32_t pid, events, paints; uint8_t state; uint8_t reserved[3]; };
struct CmMessage { uint32_t handle; uint16_t message; int16_t reserved; int32_t wparam; int32_t lparam; };

static struct CmObject cm_objects[CM_OBJECT_LIMIT];
static struct CmFile cm_files[CM_OBJECT_LIMIT];
static struct CmTask cm_tasks[CM_APP_LIMIT];
static struct CmMessage cm_queue[CM_QUEUE_LIMIT];
static uint32_t cm_window_handles[CM_APP_LIMIT];
static int cm_queue_head, cm_queue_size;
static uint32_t cm_serial = 1000;
static uint32_t cm_dropped;

static void cm_copy_name(char *dest, const char *src, int count) {
    int i = 0;
    while (i < count - 1 && src[i]) { dest[i] = src[i]; i++; }
    dest[i] = 0;
}

static uint32_t cm_handle_make(int slot) {
    return ((uint32_t)cm_objects[slot].gen << 16) | (uint32_t)(slot + 1);
}

static int cm_handle_slot(uint32_t handle, int type) {
    uint32_t number = handle & 0xffffu;
    if (number == 0 || number > CM_OBJECT_LIMIT || !(handle >> 16)) return -1;
    int slot = (int)number - 1;
    if (cm_objects[slot].gen != (uint16_t)(handle >> 16) || cm_objects[slot].type != type) return -1;
    return slot;
}

static uint32_t cm_object_alloc(int type, int owner) {
    if (owner < 0 || owner >= CM_APP_LIMIT) return 0;
    for (int i = 0; i < CM_OBJECT_LIMIT; i++) {
        if (cm_objects[i].type) continue;
        uint16_t gen = (uint16_t)(cm_objects[i].gen + 1);
        if (!gen) gen = 1;
        cm_objects[i].gen = gen;
        cm_objects[i].type = (uint8_t)type;
        cm_objects[i].owner = (uint8_t)owner;
        return cm_handle_make(i);
    }
    return 0;
}

static int cm_task_start(int app) {
    if (app < 0 || app >= CM_APP_LIMIT) return 0;
    if (!cm_tasks[app].pid) cm_tasks[app].pid = ++cm_serial;
    cm_tasks[app].state = CM_TASK_RUNNING;
    return 1;
}

static uint32_t cm_window_open(int app) {
    if (app < 0 || app >= CM_APP_LIMIT) return 0;
    cm_task_start(app);
    if (cm_handle_slot(cm_window_handles[app], CM_OBJECT_WINDOW) < 0) {
        cm_window_handles[app] = cm_object_alloc(CM_OBJECT_WINDOW, app);
    }
    return cm_window_handles[app];
}

static uint32_t cm_window_handle(int app) {
    if (app < 0 || app >= CM_APP_LIMIT) return 0;
    return cm_handle_slot(cm_window_handles[app], CM_OBJECT_WINDOW) >= 0 ? cm_window_handles[app] : 0;
}

static void cm_window_close(int app, int minimized) {
    if (app < 0 || app >= CM_APP_LIMIT) return;
    cm_tasks[app].state = minimized ? CM_TASK_MINIMIZED : CM_TASK_STOPPED;
    if (minimized) return;
    uint32_t handle = cm_window_handle(app);
    if (handle) cm_objects[(handle & 0xffffu) - 1].type = 0;
    cm_window_handles[app] = 0;
}

static int cm_post(uint32_t handle, uint16_t message, int32_t wparam, int32_t lparam) {
    if (cm_handle_slot(handle, CM_OBJECT_WINDOW) < 0) return 0;
    if (cm_queue_size >= CM_QUEUE_LIMIT) { cm_dropped++; return 0; }
    for (int k = 0; k < cm_queue_size; k++) {
        int i = (cm_queue_head + k) % CM_QUEUE_LIMIT;
        if (message == CM_MSG_PAINT && cm_queue[i].handle == handle && cm_queue[i].message == message) return 1;
    }
    int index = (cm_queue_head + cm_queue_size) % CM_QUEUE_LIMIT;
    cm_queue[index] = (struct CmMessage){handle, message, 0, wparam, lparam};
    cm_queue_size++;
    return 1;
}

static int cm_dispatch_one(void (*callback)(int, const struct CmMessage *)) {
    if (!cm_queue_size) return 0;
    struct CmMessage event = cm_queue[cm_queue_head];
    cm_queue_head = (cm_queue_head + 1) % CM_QUEUE_LIMIT;
    cm_queue_size--;
    int slot = cm_handle_slot(event.handle, CM_OBJECT_WINDOW);
    if (slot < 0) return 1;
    int app = cm_objects[slot].owner;
    cm_tasks[app].events++;
    if (event.message == CM_MSG_PAINT) cm_tasks[app].paints++;
    if (callback) callback(app, &event);
    return 1;
}

static int cm_pump(int budget, void (*callback)(int, const struct CmMessage *)) {
    int processed = 0;
    while (processed < budget && cm_dispatch_one(callback)) processed++;
    return processed;
}

static int cm_looks_like_disk_path(const char *path, const char **name) {
    if (!path || !*path) return 0;
    if (path[0] == 'D' || path[0] == 'd') {
        if (path[1] == ':') {
            path += 2;
            if (*path == '/' || *path == '\\') path++;
        }
    }
    if (!storage_name_valid(path)) return 0;
    *name = path;
    return 1;
}

static uint32_t cm_file_open(int owner, const char *path, uint32_t flags) {
    const char *name = NULL;
    if (!storage_ready || !(flags & (CM_OPEN_READ | CM_OPEN_WRITE)) ||
        ((flags & (CM_OPEN_CREATE | CM_OPEN_TRUNCATE)) && !(flags & CM_OPEN_WRITE)) ||
        !cm_looks_like_disk_path(path, &name)) return 0;
    if (!(flags & CM_OPEN_CREATE) && storage_slot(name) < 0) return 0;
    uint32_t handle = cm_object_alloc(CM_OBJECT_FILE, owner);
    if (!handle) return 0;
    int slot = (int)(handle & 0xffffu) - 1;
    struct CmFile *f = &cm_files[slot];
    cm_copy_name(f->name, name, 20);
    f->offset = 0;
    f->length = 0;
    f->changed = 0;
    f->flags = flags;
    if (storage_slot(name) >= 0 && !(flags & CM_OPEN_TRUNCATE)) {
        int count = storage_load(name, (char *)f->content, sizeof(f->content));
        if (count < 0) { cm_objects[slot].type = 0; return 0; }
        f->length = (uint32_t)count;
    } else if (flags & CM_OPEN_WRITE) {
        f->changed = 1;
    }
    return handle;
}

static int cm_file_seek(uint32_t handle, uint32_t offset) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0 || offset > STORAGE_MAX_BYTES) return 0;
    cm_files[slot].offset = offset;
    return 1;
}

static int cm_file_read(uint32_t handle, void *buffer, int count) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0 || !buffer || count < 0 || !(cm_files[slot].flags & CM_OPEN_READ)) return -1;
    struct CmFile *f = &cm_files[slot];
    if (f->offset >= f->length) return 0;
    uint32_t n = (uint32_t)count;
    if (n > f->length - f->offset) n = f->length - f->offset;
    for (uint32_t i = 0; i < n; i++) ((uint8_t *)buffer)[i] = f->content[f->offset + i];
    f->offset += n;
    return (int)n;
}

static int cm_file_write(uint32_t handle, const void *buffer, int count) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0 || !buffer || count < 0 || !(cm_files[slot].flags & CM_OPEN_WRITE)) return -1;
    struct CmFile *f = &cm_files[slot];
    if ((uint32_t)count > STORAGE_MAX_BYTES - f->offset) return -1;
    for (int i = 0; i < count; i++) f->content[f->offset + (uint32_t)i] = ((const uint8_t *)buffer)[i];
    f->offset += (uint32_t)count;
    if (f->offset > f->length) f->length = f->offset;
    f->content[f->length] = 0;
    if (count) f->changed = 1;
    return count;
}

static int cm_file_flush(uint32_t handle) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0) return 0;
    struct CmFile *f = &cm_files[slot];
    if (!f->changed) return 1;
    if (!storage_save(f->name, (const char *)f->content, f->length)) return 0;
    f->changed = 0;
    return 1;
}

static int cm_file_close(uint32_t handle) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0 || !cm_file_flush(handle)) return 0;
    cm_objects[slot].type = 0;
    return 1;
}

static int cm_file_abort(uint32_t handle) {
    int slot = cm_handle_slot(handle, CM_OBJECT_FILE);
    if (slot < 0) return 0;
    cm_objects[slot].type = 0;
    return 1;
}

static int cm_file_list(int index, char *name) { return storage_list(index, name); }

static int cm_handle_count(void) {
    int n = 0;
    for (int i = 0; i < CM_OBJECT_LIMIT; i++) if (cm_objects[i].type) n++;
    return n;
}
