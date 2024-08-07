//
// Created by Piotr on 13.05.2024.
//

#include "heap.hpp"

void Heap::init()
{
    this->pml4_entry = 0;
    this->last_pte_entry = 0;
    this->last_pde_entry = 0;
    this->last_pdpe_entry = 0;

    // Find free pml4e entry.

    // Get pml4e
    this->pml4e_pointer = vmm_make_virtual<struct pml4e*>(vmm_get_pml4());

    for (size_t i = 500; 512 > i; i++)
    {
        if (this->pml4e_pointer[i].pdpe_ptr == 0)
        {
            this->pml4_entry = i;

            break;
        }
    }

    this->base_address = vmm_create_virtual_address(false, this->pml4_entry, 0, 0, 0, 0);

    this->commit_page();
}


void Heap::commit_page()
{
    void* page = reinterpret_cast<void*>(pmm_alloc_page());

    if (page == nullptr)
    {
        kstd::printf("Failure commiting a page!\n");
        unreachable();
    }

    // map the memory
    if (this->last_pte_entry >= 512)
    {
        this->last_pte_entry = 0;
        this->last_pde_entry ++;
    }

    if (this->last_pde_entry >= 512)
    {
        this->last_pde_entry = 0;
        this->last_pdpe_entry ++;
    }

    if (this->last_pdpe_entry >= 512)
    {
        kstd::printf("Out of virtual memory!\n");
        unreachable();
    }

    // page it in
    uint64_t va = vmm_create_virtual_address(false, this->pml4_entry, this->last_pdpe_entry, this->last_pde_entry, this->last_pte_entry, 0);
    // kstd::printf("[HEAP] Entry: %llx %llx %llx %llx\n",this->pml4_entry, this->last_pdpe_entry, this->last_pde_entry, this->last_pte_entry );
    bool ok = vmm_map(
            this->pml4e_pointer,
            va,
            reinterpret_cast<uint64_t>(page),
            PROT_SUPERVISOR | PROT_RW,
            MAP_PRESENT,
            MISC_INVLPG
    );

    if (!ok)
    {
        kstd::printf("Failed to map memory to heap.\n");
        bochs_breakpoint();

        unreachable();
    }

    this->last_pte_entry ++;

    // tag the memory
    struct malloc_tag* tag = reinterpret_cast<struct malloc_tag*>(va);

    tag->state = MALLOC_STATE_FREE;
    tag->length = 4096 - sizeof (struct malloc_tag);

    // update the commitment memory.
    this->available_memory += (4096 - sizeof(struct malloc_tag));
    this->committed_memory_pages ++;
    this->entry_count ++;

    merge_blocks();
}

void Heap::print_memory_entries()
{
    uintptr_t current_address = this->base_address;

    // Iterate through memory for each entry
    for (size_t i = 0; i < this->entry_count; i++)
    {
        // Cast the current address to a malloc_tag pointer
        struct malloc_tag* tag = reinterpret_cast<struct malloc_tag*>(current_address);

        // Print details of the current entry
        kstd::printf("Memory Entry at Address: %p\n", reinterpret_cast<void*>(current_address));
        kstd::printf("    State: %d\n", tag->state);
        kstd::printf("    Length: %ld bytes\n", tag->length);

        // Move to the next memory entry
        current_address += tag->length + sizeof(struct malloc_tag);
    }
}

void Heap::merge_block() {
    uintptr_t current_address = this->base_address;

    // Iterate through memory for each entry
    for (size_t i = 0; i < this->entry_count - 1; i++) {
        struct malloc_tag *tag = reinterpret_cast<struct malloc_tag *>(current_address);
        struct malloc_tag *next_tag = reinterpret_cast<struct malloc_tag *>(reinterpret_cast<uintptr_t>(tag) + sizeof(struct malloc_tag) + tag->length); // Corrected next_tag calculation

        if (current_address >= committed_memory_pages * 4096 + base_address) return;
        if (reinterpret_cast<uint64_t>(tag) + tag->length + sizeof(struct malloc_tag) >= committed_memory_pages * 4096 + base_address) return;

        if (tag->state == MALLOC_STATE_FREE && next_tag->state == MALLOC_STATE_FREE)
        {
            tag->length += sizeof(struct malloc_tag) + next_tag->length;
            tag->state = MALLOC_STATE_FREE; // Set current tag state to free
            this->entry_count --;

            return;
        }

        current_address += sizeof(struct malloc_tag) + tag->length; // Adjusting the address correctly
    }
}

void* Heap::alloc_normal(size_t len)
{
    // Calculate size of tag structure once
    size_t tag_size = sizeof(struct malloc_tag);

    if (len > heap_get_available_memory()) {
        for (size_t i = 0; (len + 4095) / 4096 > i; i++) {
            commit_page();
        }
    }

    try_again:
    // Get the first entry.
    struct malloc_tag* this_tag = reinterpret_cast<struct malloc_tag*>(this->base_address);

    // Align length to a multiple of 32
    len += (32 - (len % 32)) % 32;

    // Iterate through the tags to find a suitable free block.
    for (size_t i = 0; i < this->entry_count; i++)
    {
        if (this_tag->state == MALLOC_STATE_FREE && this_tag->length >= (len + tag_size))
        {
            // Found a suitable free block.
            // Record the original length.
            uint64_t original_length = this_tag->length;

            // Adjust the length of the current tag to reflect the allocated block.
            this_tag->length = len;
            this_tag->state = MALLOC_STATE_USED;

            // Calculate the address for the new tag.
            struct malloc_tag* new_tag = reinterpret_cast<struct malloc_tag*>(reinterpret_cast<uintptr_t>(this_tag) + len + tag_size);

            // Set up the new tag with the remaining length.
            new_tag->length = original_length - len - tag_size;
            new_tag->state = MALLOC_STATE_FREE;

            // Update entry count to reflect the new free block.
            this->entry_count++;

            // Update available and used memory.
            this->available_memory -= (len + tag_size);
            this->used_memory += (len + tag_size);


            // Return the address after the current tag's metadata.
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this_tag) + tag_size);
        }

        // Move to the next tag.
        this_tag = reinterpret_cast<struct malloc_tag*>(reinterpret_cast<uintptr_t>(this_tag) + this_tag->length + tag_size);
    }

    commit_page();
    goto try_again;
}

void Heap::free(void* ptr)
{
    // Check if ptr is nullptr
    if (ptr == nullptr) {
        return; // Nothing to free
    }

    // Calculate the address of the corresponding malloc_tag
    struct malloc_tag* tag_to_free = reinterpret_cast<struct malloc_tag*>(reinterpret_cast<uint64_t>(ptr) - sizeof(struct malloc_tag));

    // Mark the tag as free
    tag_to_free->state = MALLOC_STATE_FREE;

    // Update used_memory and available_memory
    this->used_memory -= (tag_to_free->length + sizeof(struct malloc_tag));
    this->available_memory += (tag_to_free->length + sizeof(struct malloc_tag));

    // kstd::printf("Freed memory: %llx.\n", ptr);
}

static Heap heap;

void merge_blocks() {
    size_t current_blk_count = heap.entry_count;
    size_t this_blk_count = 0;
    while (current_blk_count != this_blk_count) {
        heap.merge_block();
        this_blk_count = current_blk_count;
        current_blk_count = heap.entry_count;
    }
}

void heap_print_entries()
{
    heap.print_memory_entries();
}

void* kmalloc(size_t len)
{
    return heap.alloc_normal(len);
}

void kfree(void* ptr)
{
    heap.free(ptr);
}

void commit_page()
{
    heap.commit_page();
}

void heap_init()
{
    heap.init();
}


uint64_t heap_get_available_memory()
{
    return heap.available_memory;
}

uint64_t heap_get_used_memory()
{
    return heap.used_memory;
}

uint64_t heap_get_used_virtual_memory()
{
    return heap.committed_memory_pages * 4096;
}