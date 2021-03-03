#ifndef _LIBC_ERRNO_H
#define _LIBC_ERRNO_H

#include "../../../kernel/include/libkern/errno.h"

extern int errno;

#define set_errno(x) (errno = x)

#endif