#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

#define SEG_MAX_SIZE 65536
//#define SEG_MAX_SIZE 131072

static uintptr_t loader(PCB *pcb, const char *filename) {
    assert(filename != NULL);

    int fd = fs_open(filename, 0, 0);
    assert(fd != -1);

    Elf_Ehdr elf_head;

    fs_read(fd, (void *)(&elf_head), sizeof(elf_head));
    //ramdisk_read((void *)(&elf_head), 0, sizeof(elf_head));

    assert(*(uint32_t *)elf_head.e_ident == 0x464c457f);
    assert(EXPECT_TYPE == elf_head.e_machine);

    uint32_t phoff = elf_head.e_phoff;
    uint32_t phsize = elf_head.e_phentsize;
    uint32_t phnum = elf_head.e_phnum;
    uint32_t entry = elf_head.e_entry;

    for (uint32_t i = 0; i < phnum; i ++) {
        Elf_Phdr pro_head;

        fs_lseek(fd, phoff + i * phsize, SEEK_SET);
        fs_read(fd, (void *)(&pro_head), sizeof(pro_head));
        //ramdisk_read((void *)(&pro_head), phoff + i * phsize, sizeof(pro_head));
        if (pro_head.p_type == PT_LOAD) {
            uintptr_t pvaddr = pro_head.p_vaddr;
            uint32_t poff = pro_head.p_offset;
            uint32_t pfilesz = pro_head.p_filesz;
            uint32_t pmemsz = pro_head.p_memsz;

            char buffer[SEG_MAX_SIZE];
            assert(SEG_MAX_SIZE > pfilesz);
            fs_lseek(fd, poff, SEEK_SET);
            //printf("loader segment size of %s: %u\n", filename, pfilesz);
            fs_read(fd, (void *)buffer, pfilesz);

            memcpy((void *)pvaddr, (void *)buffer, pmemsz);
            memset((void *)(pvaddr + pfilesz), 0, pmemsz - pfilesz);
        }
    }
    return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void(*)())entry) ();
}

