#include <common.h>
#include "syscall.h"
#include <fs.h>

int sys_exit();
uintptr_t sys_yield();
int sys_open(const char *pathname, int flags, int mode);
size_t sys_read(int fd, void *buf, size_t count);
size_t sys_write(int fd, const void *buf, size_t count);
int sys_close(int fd);
size_t sys_lseek(int fd, size_t offset, int whence);
size_t sys_sbrk(intptr_t increment);


#define STRACE_MAX 1024

static size_t i_strace = 0;
static char strace[STRACE_MAX][128];

void show_strace() {
    putstr("\n************************************************\n");
    putstr("THE STRACE CONTENTS:\n");
    for (int i = 0; i < i_strace; i ++) {
        int j = 0;
        while (strace[i][j] != '\0') {
            if (strace[i][j] == '\n') {
                putstr("\\n");
            } else {
                putch(strace[i][j]);
            }
            j ++;
        }
        putch('\n');
    }
}

static void add_strace(Context *c) {
    uintptr_t a[4] = {c->GPR1, c->GPR2, c->GPR3, c->GPR4};

    switch (a[0]) {
        case SYS_exit:
            sprintf(strace[i_strace], "exit()");
            break;
        case SYS_yield:
            sprintf(strace[i_strace], "yield()");
            break;
        case SYS_open:
            sprintf(strace[i_strace], "sys_open(%s, %d, %d)", (char *)a[1], (int)a[2], (int)a[3]);
            break;
        case SYS_read:
            sprintf(strace[i_strace], "sys_read(%d, %p, %u)", (int)a[1], (void *)a[2], (size_t)a[3]);
            break;
        case SYS_write:
            sprintf(strace[i_strace], "sys_write(%d, %p, %u)", (int)a[1], (char *)a[2], (size_t)a[3]);
            break;
        case SYS_close:
            sprintf(strace[i_strace], "sys_close(%d)", (int)a[1]);
            break;
        case SYS_lseek:
            sprintf(strace[i_strace], "sys_lseek(%d, %u, %d)", (int)a[1], (size_t)a[2], (int)a[3]);
            break;
        case SYS_brk:
            sprintf(strace[i_strace], "sys_sbrk(%u)", a[1]);
            break;
        default: 
            panic("Unhandled syscall ID = %d", a[0]);
    }
    i_strace ++;
}

void do_syscall(Context *c) {
    uintptr_t a[4] = {c->GPR1, c->GPR2, c->GPR3, c->GPR4};

    uintptr_t ret;
    switch (a[0]) {
        case SYS_exit:
            halt(sys_exit());
            break;
        case SYS_yield:
            ret = sys_yield();
            break;
        case SYS_open:
            ret = sys_open((const char *)a[1], (int)a[2], (int)a[3]);
            break;
        case SYS_read:
            ret = sys_read((int)a[1], (void *)a[2], (size_t)a[3]);
            break;
        case SYS_write:
            /* printf("before do_syscall c->GPR3: ");*/
            /* putstr((char *)a[2]);*/
            /* putch('\n');*/
            ret = sys_write((int)a[1], (const void *)a[2], (size_t)a[3]);
            //printf("after do_syscall c->GPR2: %d\n", c->GPR2);
            break;
        case SYS_close:
            ret = sys_close((int)a[1]);
            break;
        case SYS_lseek:
            ret = sys_lseek((int)a[1], (size_t)a[2], (int)a[3]);
            break;
        case SYS_brk:
            ret = sys_sbrk(a[1]);
            break;
        default: 
            panic("Unhandled syscall ID = %d", a[0]);
    }
    add_strace(c);
    c->GPRx = ret;
}

int sys_exit() {
    show_strace();
    return 0;
}

uintptr_t sys_yield() {
    yield();
    return 0;
}

int sys_open(const char *pathname, int flags, int mode) {
    return fs_open(pathname, flags, mode);
}

size_t sys_read(int fd, void *buf, size_t count) {
    return fs_read(fd, buf, count);
}

size_t sys_write(int fd, const void *buf, size_t count) {
    int len = -1;
    if (fd == 1 || fd == 2) {
        len = 0;
        char *buffer = (char *)buf;
        for (int i = 0; i < count; i ++) {
            putch(buffer[i]);
            len ++;
        }
    } else {
        len = fs_write(fd, buf, count);
    }

    return len;
}

int sys_close(int fd) {
    return fs_close(fd);
}

size_t sys_lseek(int fd, size_t offset, int whence) {
    return fs_lseek(fd, offset, whence);
}

size_t sys_sbrk(intptr_t increment) {
    return 0;
}
