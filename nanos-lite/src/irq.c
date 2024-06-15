#include <common.h>
#include "syscall.h"
void do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
    switch (e.event) {
        case EVENT_YIELD:
            printf("yield occured!\n");
            break;
        case EVENT_SYSCALL:
            //printf("syscall occured!\n");
            do_syscall(c);
            break;
        default: 
            panic("Unhandled event ID = %d", e.event);
    }
  
    return c;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    //printf("init_irq user_handler: %p\n", do_event);
    cte_init(do_event);
}
