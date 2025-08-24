#include <elf.h>
#include <lib/string.h>
#include <limine.h>
#include <sys/symbol.h>

__attribute__((used, section(".limine_requests"))) static volatile struct limine_executable_file_request kernel_file_request = {
    .id = LIMINE_EXECUTABLE_FILE_REQUEST,
    .revision = 0
};

uintptr_t symbol_lookup(const char *name) {
    // Get the address of the kernel
    void *file = kernel_file_request.response->executable_file->address;

    // Parse the ELF
    Elf64_Ehdr *ehdr = file;
    Elf64_Shdr *shdr = file + ehdr->e_shoff;
    Elf64_Sym *symtab = nullptr;
    Elf64_Xword symtab_count = 0;
    const char *strtab = nullptr;
    // Find the symbol table and the string table
    // TODO: Optimize this, perchance
    // Check if the symtab and strtab are null
    // So we can break from the loop early if we find them earlier
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        // Skip the section header string table
        if (i == ehdr->e_shstrndx) {
            continue;
        }
        Elf64_Shdr *entry = &shdr[i];
        switch (entry->sh_type) {
        case SHT_STRTAB:
            strtab = file + entry->sh_offset;
            break;
        case SHT_SYMTAB:
            symtab = file + entry->sh_offset;
            symtab_count = entry->sh_size / entry->sh_entsize;
            break;
        }
    }

    // Perform the symbol lookup
    for (Elf64_Xword i = 0; i < symtab_count; i++) {
        Elf64_Sym *symbol = &symtab[i];
        const char *symbol_name = strtab + symbol->st_name;
        if (strcmp(symbol_name, name) == 0) {
            return symbol->st_value;
        }
    }
    // We didn't find the symbol
    return 0;
}
