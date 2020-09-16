/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/display.h>
#include <errno.h>
#include <io/sockets/local_socket.h>
#include <log.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>

#define LOCAL_SOCKET_DEBUG

static file_ops_t local_socket_ops = {
    local_socket_can_read,
    local_socket_can_write,
    local_socket_read,
    local_socket_write,
    0,
    0,
    0,
    0,
    0,
    0
};

int local_socket_create(int type, int protocol, file_descriptor_t* fd)
{
    return socket_create(PF_LOCAL, type, protocol, fd, &local_socket_ops);
}

bool local_socket_can_read(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_read(&sock_entry->buffer) != 0;
}

int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t read = ringbuffer_read(&sock_entry->buffer, buf, len);
    return !(read == len);
}

bool local_socket_can_write(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_write(&sock_entry->buffer) != 0;
}

int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t written = ringbuffer_write(&sock_entry->buffer, buf, len);
    return !(written == len);
}

int local_socket_bind(file_descriptor_t* sock, char* name, uint32_t len)
{
    proc_t* p = RUNNIG_THREAD->process;

    uint32_t file_mode = S_IFSOCK | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    vfs_create(p->cwd, name, len, file_mode);
    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(p->cwd, name, &bind_dentry);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Bind: can't find path to file : %d pid\n", p->pid);
#endif
        return res;
    }
    res = vfs_open(bind_dentry, &sock->sock_entry->bind_file, O_RDONLY);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Bind: can't open file : %d pid\n", p->pid);
#endif
        return res;
    }
#ifdef LOCAL_SOCKET_DEBUG
    log("Bind local socket at %x : %d pid", sock->sock_entry, p->pid);
#endif
    sock->sock_entry->bind_file.dentry->sock = sock->sock_entry;
    return 0;
}

int local_socket_connect(file_descriptor_t* sock, char* name, uint32_t len)
{
    proc_t* p = RUNNIG_THREAD->process;

    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(p->cwd, name, &bind_dentry);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: can't find path to file : %d pid\n", p->pid);
#endif
        return res;
    }
    if ((bind_dentry->inode->mode & S_IFSOCK) == 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: file not a socket : %d pid\n", p->pid);
#endif
        return -ENOTSOCK;
    }

    if (!bind_dentry->sock) {
        return -EBADF;
    }
    sock->sock_entry = bind_dentry->sock;
#ifdef LOCAL_SOCKET_DEBUG
    kprintf("Connected to local socket at %x : %d pid", bind_dentry->sock, p->pid);
#endif
    return 0;
}