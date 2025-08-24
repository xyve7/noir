#include <elf.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <sys/symbol.h>
#include <task/loader.h>

static inline bool elf_is_magic(Elf64_Ehdr *ehdr) {
    return memcmp(ehdr->e_ident, "\x7F\x45\x4c\x46", 4) == 0;
}
uint64_t elf_prot_to_vmm(Elf64_Word flags) {
    uint64_t vmm_flags = 0;
    if (flags & PF_W) {
        vmm_flags |= VMM_WRITE;
    }
    // No execution, we disable
    if ((flags & PF_X) == 0) {
        vmm_flags |= VMM_XD;
    }
    if (flags & PF_R) {
        vmm_flags |= VMM_PRESENT;
    }
    return vmm_flags;
}
uintptr_t elf_load_no_pie(page_table *pt, void *buffer) {
    Elf64_Ehdr *ehdr = buffer;
    Elf64_Phdr *phdr = buffer + ehdr->e_phoff;
    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *entry = &phdr[i];
        if (entry->p_type != PT_LOAD) {
            continue;
        }
        // We get the size in pages
        size_t page_count = ALIGN_UP(entry->p_memsz, PAGE_SIZE) / PAGE_SIZE;
        // Copy the data over
        // But before we copy the pages over, some LOAD segments dont start at 4KiB boundaries
        // So we get the offset
        void *pages = pmm_alloc_zeroed(page_count);
        uintptr_t page_offset = entry->p_vaddr & 0x0FFF;

        memcpy(VIRT(pages) + page_offset, buffer + entry->p_offset, entry->p_filesz);
        // Map the pages correctly
        // We do it according to the virtual address, aligned down
        uintptr_t virt_start = entry->p_vaddr;
        if (page_offset != 0) {
            virt_start = ALIGN_DOWN(entry->p_vaddr, page_offset);
        }
        uint64_t flags = elf_prot_to_vmm(entry->p_flags);
        for (size_t i = 0; i < page_count; i++) {
            vmm_map(pt, (uintptr_t)pages, virt_start + (i * page_count), flags | VMM_USER);
        }
    }
    return ehdr->e_entry;
}
uintptr_t elf_load_pie(page_table *pt, void *buffer) {
    // This isnt much different, we just define a base address
    uintptr_t base = 0x400000;
    Elf64_Ehdr *ehdr = buffer;
    Elf64_Phdr *phdr = buffer + ehdr->e_phoff;
    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *entry = &phdr[i];
        if (entry->p_type != PT_LOAD) {
            continue;
        }
        // We get the size in pages
        size_t page_count = ALIGN_UP(entry->p_memsz, PAGE_SIZE) / PAGE_SIZE;
        // Copy the data over
        // But before we copy the pages over, some LOAD segments dont start at 4KiB boundaries
        // So we get the offset
        void *pages = pmm_alloc_zeroed(page_count);
        uintptr_t page_offset = entry->p_vaddr & 0x0FFF;

        memcpy(VIRT(pages) + page_offset, buffer + entry->p_offset, entry->p_filesz);
        // Map the pages correctly
        // We do it according to the virtual address, aligned down
        uintptr_t virt_start = base + entry->p_vaddr;
        if (page_offset != 0) {
            virt_start = base + ALIGN_DOWN(entry->p_vaddr, page_offset);
        }
        uint64_t flags = elf_prot_to_vmm(entry->p_flags);
        for (size_t i = 0; i < page_count; i++) {
            vmm_map(pt, (uintptr_t)pages, virt_start + (i * page_count), flags | VMM_USER);
        }
    }
    return base + ehdr->e_entry;
}

uintptr_t elf_load_exec(page_table *pt, void *buffer) {
    Elf64_Ehdr *ehdr = buffer;

    if (!elf_is_magic(ehdr)) {
        PANIC("Not an ELF file, MAGIC missing!");
    }
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        PANIC("Not a 64-bit ELF!");
    }
    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        PANIC("Not little-endian!");
    }
    if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
        PANIC("Not System-V ABI!");
    }
    if (ehdr->e_machine != EM_X86_64) {
        PANIC("Not AMD64!");
    }

    // We check if its a DYN (PIE Executable), or a standard EXEC
    // I might just merge these
    if (ehdr->e_type == ET_EXEC) {
        return elf_load_no_pie(pt, buffer);
    } else if (ehdr->e_type == ET_DYN) {
        return elf_load_pie(pt, buffer);
    } else {
        PANIC("Unsupported type, only supports ET_EXEC and ET_DYN!");
    }
}
uintptr_t UNOPTIMIZED elf_load_rel(void *file) {
    // We make a few assumptions, the first being that the module is compiled with -mcmodel=large
    // NOTE: Yes, I loop over this like a dozen times, I'm well aware.
    // I'd rather write the terrible code than mull over how to make this efficient and end up never writing it.
    // TODO: Make this more efficient?

    // Calculate the size
    // We want every section to have its own page, for permission purposes
    // (Even though we don't do any permission stuff right now, we will later)
    // We calculate the size by aligning up to the page size, so we avoid putting everything on one page or something.
    Elf64_Ehdr *ehdr = file;
    Elf64_Shdr *shdr = file + ehdr->e_shoff;
    size_t total_size = 0;
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if ((shdr[i].sh_type == SHT_PROGBITS || shdr[i].sh_type == SHT_NOBITS) && shdr[i].sh_size > 0) {
            total_size += ALIGN_UP(shdr[i].sh_size, PAGE_SIZE);
        }
    }
    size_t page_count = total_size / PAGE_SIZE;
    void *pages = pmm_alloc_zeroed(page_count);
    uintptr_t base = (uintptr_t)VIRT(pages);
    // Now that we have allocated the pages
    // We modify the elf and give the section headers actual addresses
    // This is so the relocations actually point to something later
    // We make sure the header is actually relevant, meaning either SHT_PROGBITS or SHT_NOBITS.
    // AND, it actually has a size
    // Also while we are here, grab the symbol table
    size_t page_index = 0;
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if ((shdr[i].sh_type == SHT_PROGBITS || shdr[i].sh_type == SHT_NOBITS) && shdr[i].sh_size > 0) {
            shdr[i].sh_addr = base + (page_index * PAGE_SIZE);
            page_index++;
        }
    }

    // Okay, one more iteration before we do the relocations, lets fix the symbols
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type != SHT_SYMTAB) {
            continue;
        }
        Elf64_Shdr *section = &shdr[i];
        const char *strtab = file + shdr[section->sh_link].sh_offset;
        Elf64_Sym *symtab = file + section->sh_offset;
        for (Elf64_Half j = 0; j < (section->sh_size / section->sh_entsize); j++) {
            // We first check if there even is a symbol, if there is, we check if its defined
            // If it isn't its likely refering to a kernel symbol, so we replace it
            Elf64_Sym *sym = &symtab[j];
            // Check if the symbol is defined
            if (sym->st_shndx == SHN_UNDEF) {
                const char *symbol_name = strtab + sym->st_name;
                sym->st_value = symbol_lookup(symbol_name);
            } else {
                // Align the address properly
                sym->st_value += shdr[sym->st_shndx].sh_addr;
            }
        }
    }

    // Now that the addresses are found, we look for the relocation tables
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_RELA) {
            // Find where the relocations are
            Elf64_Shdr *relsec = &shdr[i];
            Elf64_Rela *reltab = file + shdr[i].sh_offset;
            // Iterate through every relocation
            for (Elf64_Half j = 0; j < (shdr[i].sh_size / shdr[i].sh_entsize); j++) {
                // Get all the information
                // Offset into the SECTION
                // sh_info stores where the relocation applies
                // sh_link stores where the symbol table is
                // shdr[relsec->sh_link] -> symbol table section header (symsec)
                Elf64_Shdr *symsec = &shdr[relsec->sh_link];
                Elf64_Shdr *section = &shdr[relsec->sh_info];

                Elf64_Sym *symtab = file + symsec->sh_offset;
                void *offset = file + shdr[relsec->sh_info].sh_offset + reltab[j].r_offset;
                Elf64_Word symbol_index = ELF64_R_SYM(reltab[j].r_info);
                Elf64_Half type = ELF64_R_TYPE(reltab[j].r_info);
                Elf64_Sym *sym = &symtab[symbol_index];

                // Now do the relocation
                switch (type) {
                case R_X86_64_64:
                    *((Elf64_Xword *)offset) = sym->st_value + reltab[j].r_addend;
                    break;
                case R_X86_64_PC32:
                    *((Elf64_Word *)offset) = sym->st_value + reltab[j].r_addend - reltab[j].r_offset;
                    break;
                case R_X86_64_RELATIVE:
                    *((Elf64_Addr *)offset) = section->sh_addr + reltab[j].r_addend;
                    break;
                }
            }
        }
    }

    // Finally, lets copy everything
    page_index = 0;
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if ((shdr[i].sh_type == SHT_PROGBITS || shdr[i].sh_type == SHT_NOBITS) && shdr[i].sh_size > 0) {
            memcpy(VIRT(pages) + (page_index * PAGE_SIZE), file + shdr[i].sh_offset, shdr[i].sh_size);
            page_index++;
        }
    }

    // Temporary, will remove later
    Elf64_Addr address = 0;
    for (Elf64_Half i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            const char *strtab = file + shdr[shdr[i].sh_link].sh_offset;
            Elf64_Sym *symtab = file + shdr[i].sh_offset;
            for (Elf64_Half j = 0; j < (shdr[i].sh_size / shdr[i].sh_entsize); j++) {
                if (strcmp(strtab + symtab[j].st_name, "init") == 0) {
                    address = symtab[j].st_value;
                    break;
                }
            }
        }
    }
    return address;
}
uintptr_t elf_load_module(void *buffer) {
    Elf64_Ehdr *ehdr = buffer;

    if (!elf_is_magic(ehdr)) {
        PANIC("Not an ELF file, MAGIC missing!");
    }
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        PANIC("Not a 64-bit ELF!");
    }
    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        PANIC("Not little-endian!");
    }
    if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
        PANIC("Not System-V ABI!");
    }
    if (ehdr->e_machine != EM_X86_64) {
        PANIC("Not AMD64!");
    }

    // We check if its a REL (Module)
    // I might just merge these
    if (ehdr->e_type == ET_REL) {
        return elf_load_rel(buffer);
    } else {
        PANIC("Unsupported type, only supports ET_REL!");
    }
}
