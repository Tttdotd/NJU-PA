#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t count);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t count);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t count) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t count) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode) {
    int len = sizeof(file_table)/sizeof(Finfo);
    for (int i = 0; i < len; i ++) {
        if (strcmp(file_table[i].name, pathname) == 0) {
            file_table[i].open_offset = 0;
            return i;
        }
    }
    return -1;
}

int fs_close(int fd) {
    return 0;
}

size_t fs_read(int fd, void *buf, size_t count) {
    size_t size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    size_t open_offset = file_table[fd].open_offset;

    size_t remain = size - open_offset;
    assert(remain != 0);

    size_t len = count > remain ? remain : count;

    ramdisk_read(buf, disk_offset + open_offset, len);
    file_table[fd].open_offset += len;

    return len;
}

size_t fs_write(int fd, const void *buf, size_t count) {
    size_t size = file_table[fd].size;
    size_t disk_offset = file_table[fd].disk_offset;
    size_t open_offset = file_table[fd].open_offset;

    size_t remain = size - open_offset;
    assert(remain != 0);
    
    size_t len = count > remain ? remain : count;

    ramdisk_write(buf, disk_offset + open_offset, len);
    file_table[fd].open_offset += len;

    return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
    size_t size = file_table[fd].size;

    if (whence == SEEK_SET) {
        assert(offset <= size);
        file_table[fd].open_offset = offset;
        return offset;
    } else if (whence == SEEK_CUR) {
        size_t open_offset = file_table[fd].open_offset;
        size_t target_offset = open_offset + offset;
        assert(target_offset <= size);
        file_table[fd].open_offset = target_offset;
        return target_offset;
    } else if (whence == SEEK_END) {
        size_t target_offset = size + offset;
        assert(target_offset <= size);
        file_table[fd].open_offset = target_offset;
        return target_offset;
    }

    return (size_t)-1;
}
