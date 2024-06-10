#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;

    switch (a[0]) {
        case SYS_yield:
            c->GPRx = sys_yield();
            break;
        case SYS_exit:
            halt(0);
            break;
        case SYS_write:
            c->GPRx = sys_write(c->GPR2, (const void *)c->GPR3, c->GPR4);
            break;
        case SYS_brk:
            c->GPRx = sys_sbrk(c->GPR2);
            break;
        default: 
            panic("Unhandled syscall ID = %d", a[0]);
    }
}

uintptr_t sys_yield() {
    yield();
    return 0;
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
    }

    return len;
}

size_t sys_sbrk(intptr_t increment) {
    return 0;
}
