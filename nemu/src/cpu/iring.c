#include <cpu/cpu.h>
#ifdef CONFIG_IRING

IRingBuf iring_buf;

void init_iring_buf() {
    iring_buf.front = 0;
    iring_buf.length = 0;
    iring_buf.tail = 0;
}

void add_iring_buf(char *src) {
    strcpy(iring_buf.buf[iring_buf.tail], src);
    iring_buf.tail = (iring_buf.tail + 1) % SIZE_IRINGBUF;
    if (iring_buf.length != SIZE_IRINGBUF)
        iring_buf.length ++;
    if (iring_buf.front == iring_buf.tail)
        iring_buf.front = (iring_buf.front + 1) % SIZE_IRINGBUF;
}

void print_iring_buf() {
    int i;
    log_write("Puts the iring_buf:\n");
    for (i = 0; i < iring_buf.length; i++) {
        log_write("%s\n", iring_buf.buf[(iring_buf.front + i)%SIZE_IRINGBUF]);
    }
}
#endif
