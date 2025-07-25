#include <cpu/cpu.h>
#include <kernel.h>
#include <stdint.h>
#include <sys/gdt.h>

typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
} gdtr;

typedef struct [[gnu::packed]] {
    uint32_t reserved;
    uint64_t rsp0;
    uint8_t ignored[92];
} tss_entry;

typedef struct [[gnu::packed]] {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1_flags;
    uint8_t base2;
    uint32_t base3;
    uint32_t reserved;
} gdt_long_entry;

typedef struct [[gnu::packed]] {
    uint64_t gdt[6];
    gdt_long_entry tss;
} gdt_tss;

gdt_long_entry gdt_new_long_entry(uint32_t limit, uint64_t base, uint8_t access, uint8_t flags) {
    gdt_long_entry e;
    e.limit0 = limit & 0xFFFF;
    e.base0 = base & 0xFFFF;
    e.base1 = (base >> 16) & 0xFF;
    e.access = access;
    e.limit1_flags = (flags << 4) | ((limit >> 16) & 0x0F);
    e.base2 = (base >> 24) & 0xFF;
    e.base3 = (base >> 32) & 0xFFFFFFFF;
    e.reserved = 0;
    return e;
}

tss_entry tss;
gdt_tss gdt_and_tss;
extern void gdt_load_tss();
extern void gdt_reload_seg();
extern uint64_t gdt_get_rsp();
void gdt_set_rsp0(uint64_t rsp0) {
    tss.rsp0 = rsp0;
}
void gdt_init() {

    // Since the GDT is irrelavent in long mode aside from the TSS
    // We will just hardcode it
    gdt_and_tss.gdt[0] = 0x0;
    gdt_and_tss.gdt[1] = 0x00AF9A000000FFFF;
    gdt_and_tss.gdt[2] = 0x00CF92000000FFFF;
    gdt_and_tss.gdt[3] = 0x00AFFA000000FFFF;
    gdt_and_tss.gdt[4] = 0x00CFF2000000FFFF;
    // We create duplicate entry because the SYSCALL instruction
    // for some reason does + 16 on the CS given in the IA32_STAR MSR
    // I would seriously love to know the rationale behind this
    // but to me this seems like a moron made this.
    gdt_and_tss.gdt[5] = 0x00AFFA000000FFFF;
    tss.rsp0 = gdt_get_rsp();
    gdt_and_tss.tss = gdt_new_long_entry(sizeof(tss) - 1, (uint64_t)&tss, 0x89, 0);

    gdtr r;
    r.limit = sizeof(gdt_and_tss) - 1;
    r.base = (uint64_t)&gdt_and_tss;

    asm("lgdt %0" : : "m"(r));
    gdt_reload_seg();
    gdt_load_tss();

    LOG("GDT Initialized");
}
