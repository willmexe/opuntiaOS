/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VMM_H
#define _KERNEL_MEM_VMM_H

#include <libkern/libkern.h>
#include <mem/bits/mmu.h>
#include <mem/bits/swap.h>
#include <mem/bits/vm.h>
#include <mem/pmm.h>
#include <platform/generic/vmm/consts.h>
#include <platform/generic/vmm/pde.h>
#include <platform/generic/vmm/pte.h>

#define vmm_is_kernel_address(add) (add >= KERNEL_BASE)

#define IS_KERNEL_VADDR(vaddr) (vaddr >= KERNEL_BASE)
#define IS_USER_VADDR(vaddr) (vaddr < KERNEL_BASE)

struct memzone;
struct vm_ops {
    int (*load_page_content)(struct memzone* zone, uintptr_t vaddr);
    int (*swap_page_mode)(struct memzone* zone, uintptr_t vaddr);
    int (*restore_swapped_page)(struct memzone* zone, uintptr_t vaddr);
};
typedef struct vm_ops vm_ops_t;

/**
 * PUBLIC FUNCTIONS
 */

struct dynamic_array;

int vmm_setup();
int vmm_setup_secondary_cpu();

int vmm_free_pdir(pdirectory_t* pdir, struct dynamic_array* zones);

int vmm_alloc_page(uintptr_t vaddr, uint32_t settings);
int vmm_tune_page(uintptr_t vaddr, uint32_t settings);
int vmm_tune_pages(uintptr_t vaddr, size_t length, uint32_t settings);
int vmm_free_page(uintptr_t vaddr, page_desc_t* page, struct dynamic_array* zones);

int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t settings);
int vmm_map_pages(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, uint32_t settings);
int vmm_unmap_page(uintptr_t vaddr);
int vmm_unmap_pages(uintptr_t vaddr, size_t n_pages);
int vmm_copy_page(uintptr_t to_vaddr, uintptr_t src_vaddr, ptable_t* src_ptable);
int vmm_swap_page(ptable_t* ptable, struct memzone* zone, uintptr_t vaddr);

int vmm_map_page_lockless(uintptr_t vaddr, uintptr_t paddr, uint32_t settings);
int vmm_map_pages_lockless(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, uint32_t settings);
int vmm_unmap_page_lockless(uintptr_t vaddr);
int vmm_unmap_pages_lockless(uintptr_t vaddr, size_t n_pages);
int vmm_copy_page_lockless(uintptr_t to_vaddr, uintptr_t src_vaddr, ptable_t* src_ptable);

pdirectory_t* vmm_new_user_pdir();
pdirectory_t* vmm_new_forked_user_pdir();
void* vmm_bring_to_kernel(uint8_t* src, size_t length);
void vmm_prepare_active_pdir_for_writing_at(uintptr_t dest_vaddr, size_t length);
void vmm_copy_to_user(void* dest, void* src, size_t length);
void vmm_copy_to_pdir(pdirectory_t* pdir, void* src, uintptr_t dest_vaddr, size_t length);

pdirectory_t* vmm_get_active_pdir();
pdirectory_t* vmm_get_kernel_pdir();

int vmm_switch_pdir(pdirectory_t* pdir);

int vmm_page_fault_handler(uint32_t info, uintptr_t vaddr);

inline static table_desc_t* _vmm_pdirectory_lookup(pdirectory_t* pdir, uintptr_t vaddr)
{
    if (pdir) {
        return &pdir->entities[VMM_OFFSET_IN_DIRECTORY(vaddr)];
    }
    return 0;
}

inline static page_desc_t* _vmm_ptable_lookup(ptable_t* ptable, uintptr_t vaddr)
{
    if (ptable) {
        return &ptable->entities[VMM_OFFSET_IN_TABLE(vaddr)];
    }
    return 0;
}

#endif // _KERNEL_MEM_VMM_H
