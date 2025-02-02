/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_ABI_DRIVERS_H
#define _BOOT_LIBBOOT_ABI_DRIVERS_H

#include <libboot/types.h>

#define MASKDEFINE(N, P, S) \
    N##_POS = (P),          \
    N##_SIZE = (S),         \
    N##_MASK = ((~(~0 << (S))) << (P))

#define TOKEN_PASTE_IMPL(x, y) x##y
#define TOKEN_PASTE(x, y) TOKEN_PASTE_IMPL(x, y)
#define SKIP(x, y) char TOKEN_PASTE(prefix, __LINE__)[y - x - 8]

struct drive_desc {
    void* init;
    void* read;
    void* write;
};
typedef struct drive_desc drive_desc_t;

struct fs_desc {
    void* read;
};
typedef struct fs_desc fs_desc_t;

#endif // _BOOT_LIBBOOT_ABI_DRIVERS_H