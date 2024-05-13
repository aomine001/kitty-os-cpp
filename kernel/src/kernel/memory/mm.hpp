//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_MM_HPP
#define KITTY_OS_CPP_MM_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <limine.h>
#include <kstd/kstdio.hpp>
#include <kstd/kstring.hpp>
#include <kstd/kbitmap.hpp>

#define VM_PAGE_SIZE 4096

extern size_t mm_memmap_entry_count;
extern limine_memmap_entry** mm_memmap_entries;
extern limine_memmap_request mm_memap_request;
extern limine_memmap_response* mm_memmap_response;

void pmsa_initialize();
const char* mm_entry_type_to_string(uint64_t type);
uint64_t mm_align_mem(uint64_t addr, uint64_t align);
bool pmsa_is_ready();
void mm_enumerate_memmap_entries(bool compact_write = true);
uint64_t mm_physical_to_virtual_addr(uint64_t phys_addr);
void* pmsa_alloc_page();
void pmsa_free_page(void* ptr);

inline void mm_invlpg(void* ptr)
{
    asm volatile ("invlpg (%0)" ::"r"(ptr) : "memory");
}

struct cr3_reg
{
    uint64_t ignored : 3;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t ignored2 : 7;
    uint64_t pdbr : 36;
    uint64_t reserved : 16;
} __attribute((packed));

inline uint64_t read_cr3()
{
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

class MemoryBitmap : public Bitmap<uint8_t>
{
public:
    /*
     * 1 - Memory page is being used.
     * 0 - Memory page isn't being used.
     * */

    size_t CalculatePageIdx(uint64_t addr);
    void MarkAddressUsed(uint64_t addr);
    void MarkAddressUnused(uint64_t addr);
    void MarkMemoryInRange(uint64_t start_addr, uint64_t end_addr);
    void UnmarkMemoryInRange(uint64_t start_addr, uint64_t end_addr);
};

void pmsa_test();

struct VirtAddress
{
    uint64_t ones_or_zeroes : 16;
    uint64_t pml4e : 9;
    uint64_t pdpe : 9;
    uint64_t pde : 9;
    uint64_t pte : 9;
    uint64_t offset : 12;
} __attribute__((packed));
/*
 * Field entry definitions:
 * p - present bit
 *  if pt.p == 0 && pt.is_accessed:
 *      raise exception.pf
 *
 * rw - Read/Write bit, allows for reading and writing for that page.
 * us - User/Supervisor bit, This bit controls user (CPL 3) access. if 0 then cpl 0, 1, 2 has access, if 1 then cpl 3.
 * pwt - Page-Level Writethrough, his bit indicates whether the page-translation table or
physical page to which this entry points has a writeback or writethrough caching policy
 * pct - his bit indicates whether the page-translation table or
physical page to which this entry points is cacheable.
 * a - Accessed bit. This is set when the table has been accesed. Software clears it to keep track of used pages.
 * d - Dirty bit. It indicates if page has been written to.
 * g - Global page bit. This page won't be invalidated when mm system changes.
 * avl - Those bits are for ME! (Available to software)
 * pat - Page Attribute Table. (idfk what it does i dont care)
 * nx - No execute. If cleared then code can be executed. duh
 * */
struct pml4e
{
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign : 1;
    uint64_t mbz0 : 1;
    uint64_t mbz1 : 1;
    uint64_t avl : 3;
    uint64_t pdpe_base_address : 40;
    uint64_t available : 11;
    uint64_t nx : 1;
} __attribute__((packed));

struct pdpe
{
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign0 : 1;
    uint64_t z : 1; // must be zero
    uint64_t ign1 : 1;
    uint64_t avl : 3;
    uint64_t pde_base_address : 40;
    uint64_t available : 11;
    uint64_t nx : 1;
} __attribute__((packed));

struct pde
{
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t pat : 1;
    uint64_t g : 1;
    uint64_t avl : 3;
    uint64_t pte_base_address : 40;
    uint64_t available : 11;
    uint64_t nx : 1;
} __attribute__((packed));

struct pte
{
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t pat : 1;
    uint64_t g : 1;
    uint64_t avl : 3;
    uint64_t pp_base_address : 40;
    uint64_t available : 7;
    uint64_t pke : 4;
    uint64_t nx : 1;
} __attribute((packed));

#define PROT_RW 1 << 0
#define PROT_EXEC 1 << 1
#define PROT_SUPERVISOR 1 << 2
#define PROT_FORCE_MAP 1 << 7

#define MAP_PWT 1 << 3
#define MAP_PCD 1 << 4
#define MAP_PAT 1 << 5
#define MAP_GLOBAL 1 << 6
#define MAP_PRESENT 1 << 8

#define MISC_INVLPG 1 << 0

#define MAP_SUCCESS true
#define MAP_FAILURE false

uint64_t get_logical_address_pml4();

bool vmsa_map_page(
        pml4e* pml4e,
        uint64_t virt_address,
        uint64_t phys_address,
        int prot_flags,
        int map_flags,
        int misc_flags,
        uint64_t pke
);

bool vmsa_map_pages(
        pml4e* pml4e,
        uint64_t virt_address,
        uint64_t phys_address,
        uint64_t page_count,
        int prot_flags,
        int map_flags,
        int misc_flags,
        uint64_t pke
);

constexpr uint64_t mm_create_va(
        bool umode, // This specifies if we use 0x0000 (usermode) or 0xffff (supervisor)
        uint64_t pml4e,
        uint64_t pdpe,
        uint64_t pde,
        uint64_t pte,
        uint64_t offset
)
{
    uint64_t va = 0;

    if (!umode) va |= ((uint64_t)0xffff << 48);

    va |= pml4e << 39;
    va |= pdpe << 30;
    va |= pde << 21;
    va |= pte << 12;
    va |= offset;

    return va;
}

bool vmsa_cmp_pml4(
        pml4e* pml4e,
        size_t index,
        int prot_flags,
        int map_flags
);

bool vmsa_cmp_pdpe(
        pdpe* pdpe,
        size_t index,
        int prot_flags,
        int map_flags
);

bool vmsa_cmp_pde(
        pde* pde,
        size_t index,
        int prot_flags,
        int map_flags
);

bool vmsa_cmp_pte(
        pte* pte,
        size_t index,
        int prot_flags,
        int map_flags
);

uint64_t vmsa_alloc(
        pml4e* pml4e,
        size_t pml4e_index,
        int prot_flags,
        int map_flags,
        int misc_flags
);

#endif //KITTY_OS_CPP_MM_HPP