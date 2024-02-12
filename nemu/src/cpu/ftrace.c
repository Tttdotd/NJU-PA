#include <common.h>
#include <elf.h>

#define ENTRY_SIZE 1024

#define CALL 0
#define RET 1
#define CALL_FMT "0x%x: " "%s" "%s" "[%s@0x%x]"
#define RET_FMT "0x%x: " "%s" "%s" "[%s->0x%x]"

extern Elf32_Ehdr elf_head;
extern Elf32_Sym symtab[];
extern int sym_num;
extern char strtab[];

static char tab[256] = "";
static size_t tab_len = 0;

static void tab_inc() {
    for (int i = 0; i < 2; i++) {
        tab[tab_len] = ' ';
        tab_len ++;
    }
    tab[tab_len] = '\0';
}
static void tab_dec() {
    for (int i = 0; i < 2; i++) {
        tab_len --;
        tab[tab_len] = '\0';
    }
}

void add_ftrace(vaddr_t pc, int type, vaddr_t des_address) {
    char name[256] = "???";

    vaddr_t address = type == CALL ? des_address : pc;

    //map the address into a string
    for (int i = 0; i < sym_num; i ++) {
        if ((type == CALL && symtab[i].st_value == address) ||
            (type == RET && (symtab[i].st_value <= address && address < symtab[i].st_value + symtab[i].st_size))) {
            sprintf(name, "%s", strtab + symtab[i].st_name);
            break;
        }
    }

    if (strcmp(name, "???") != 0) {
        if (type == CALL) {
            log_ftrace_write(CALL_FMT"\n", pc, tab, "call ", name, des_address);
            tab_inc();
        }
        if (type == RET) {
            tab_dec();
            log_ftrace_write(RET_FMT"\n", pc, tab, "ret ", name, des_address);
        }
    }

}

