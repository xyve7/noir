#pragma once

#include <stdint.h>
typedef struct
{
    uint8_t ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf64_ehdr;

typedef struct
{
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
} elf64_shdr;

typedef struct
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} elf64_phdr;

#define ELF_ETYPE_REL 0x01
#define ELF_ETYPE_EXEC 0x02
#define ELF_ETYPE_DYN 0x03

#define ELF_PTYPE_LOAD 0x00000001

#define ELF_PFLAGS_EXEC 0x1
#define ELF_PFLAGS_WRITE 0x2
#define ELF_PFLAGS_READ 0x4

#define ELF_ECLASS_AMD64 2
#define ELF_EDATA_LE 1

#define ELF_MACHINE_AMD64 0x3E

static inline bool elf_is_elf(elf64_ehdr *ehdr) {
    return ehdr->ident[0] == 0x7F && ehdr->ident[1] == 'E' && ehdr->ident[2] == 'L' && ehdr->ident[3] == 'F';
}
static inline bool elf_is_valid(elf64_ehdr *ehdr) {
    return ehdr->ident[4] == ELF_ECLASS_AMD64 && ehdr->ident[5] == ELF_EDATA_LE && ehdr->machine == ELF_MACHINE_AMD64;
}
