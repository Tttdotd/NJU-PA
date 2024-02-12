#include <common.h>
#include <elf.h>

FILE *elf_fp = NULL;

Elf32_Ehdr elf_head;

Elf32_Sym symtab[256];
int sym_num = 0;
char strtab[1024];

void read_elfHead();
void read_shTable();
void read_sections();

void init_elf(const char *elf_file) {
    if (elf_file != NULL) {
        FILE *fp = fopen(elf_file, "rb");//binary file "rb"
        Assert(fp, "Can not open '%s'", elf_file);
        elf_fp = fp;
        Log("The ELF file is opened: %s", elf_file);

        /* read the elf head */
        read_elfHead();

        /* read some sections */
        read_sections();
        Log("Read the symtab and strtab successfully.");
    }
    else 
        Log("There is no ELF file");
}

/* void test_elfread() {*/
/*     Log("The number of the section head: %u", elf_head.e_shnum);*/
/* }*/
 
void read_elfHead() {
    if (elf_fp != NULL) {
        size_t n = fread(&elf_head, sizeof(elf_head), 1, elf_fp);
        Assert(n == 1, "Reading the ELF head failed.");
        Log("The ELF head is readed successfully"); 
        /* test_elfread();*/
    }
}

void read_sections() {
    size_t size_sh_table = elf_head.e_shnum * elf_head.e_shentsize;

    Elf32_Shdr sh_table[128];
    char shstrtab[256];

    //read the section head table
    int n = fseek(elf_fp, (long)elf_head.e_shoff, SEEK_SET);
    assert(n == 0);
    n = fread(sh_table, size_sh_table, 1, elf_fp);
    assert(n == 1);

    //read the shstrtab
    n = fseek(elf_fp, (long)sh_table[elf_head.e_shstrndx].sh_offset, SEEK_SET);
    assert(n == 0);
    n = fread(shstrtab, sh_table[elf_head.e_shstrndx].sh_size, 1, elf_fp);
    assert(n == 1);

    //read the .symtab and .strtab
    for (int i = 0; i < elf_head.e_shnum; i ++) {
        if (strcmp(".symtab", shstrtab + sh_table[i].sh_name) == 0) {
            n = fseek(elf_fp, (long)sh_table[i].sh_offset, SEEK_SET);
            assert(n == 0);
            n = fread(symtab, sh_table[i].sh_size, 1, elf_fp);
            assert(n == 1);
            sym_num = sh_table[i].sh_size / sh_table[i].sh_entsize;
        }
        if (strcmp(".strtab", shstrtab + sh_table[i].sh_name) == 0) {
            n = fseek(elf_fp, (long)sh_table[i].sh_offset, SEEK_SET);
            assert(n == 0);
            n = fread(strtab, sh_table[i].sh_size, 1, elf_fp);
            assert(n == 1);
        }
    }
}

