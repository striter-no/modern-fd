#include "methods.h"
#include <stdlib.h>

#ifndef MFD_STREAM

typedef struct {
    t_fd *tfd;
    bool  owner;
} mfd_stream;

void mfd_str_begin(t_fd *tfd, mfd_stream *out){
    out->tfd = tfd;
    out->owner = false;
}

int mfd_str_create(int rfd, bool writable, bool readable, mfd_stream *out){
    out->tfd = malloc(sizeof(t_fd));
    if (!out->tfd) return -1;

    r_fd fd = mfd_rfd(rfd, writable, readable);
    mfd_tfd(&fd, RAW_FD, out->tfd);
    out->owner = true;

    return 0;
}

int mfd_str_readexact(mfd_stream str, void *buffer, size_t bytes){
    if (str.tfd->t == PROGRAM_FD && str.tfd->fd.pfd->used < bytes) {
        fprintf(stderr, "mfd_str_readexact: do not use on pfd streams with `used < bytes`\n");
        return -2;
    } else {
        mfd_read(str.tfd, buffer, bytes);
    }

    if (!mfd_readable(*str.tfd)){
        fprintf(stderr, "mfd_str_readexact: fd is not readable\n");
        return -2;
    }

    size_t remain = bytes;
    while (remain > 0) {
        ssize_t write_len = mfd_read(str.tfd, buffer, remain);
        if (write_len < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                // EINTR: interrupted by system
                // EAGAIN: read is blocked
                // Try again
                continue;
            }
            return -1; // Unexpected error
        } else if (write_len == 0) {
            return -3; // No data from socket: disconnected.
        } else {
            // read success
            remain -= write_len;
            buffer += write_len;
        }
    }

    return 0;
}

int mfd_str_readmax(mfd_stream str, size_t *out_size, void **buffer){
    if (!mfd_readable(*str.tfd)){
        fprintf(stderr, "mfd_str_readmax: fd is not readable\n");
        return -2;
    }

    if (str.tfd->t == PROGRAM_FD){
        *buffer = malloc(mfd_available(*str.tfd));
        if (!(*buffer)) return -1;

        mfd_read(str.tfd, *buffer, str.tfd->fd.pfd->used);
        return 0;
    }
    
    size_t head = 1024;
    *buffer = malloc(head);
    *out_size = 0;
    while (true) {
        char buf[1024] = {0};
        ssize_t read_len = mfd_read(str.tfd, buf, 1024);
        if (read_len < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        } else if (read_len == 0) {
            return 0;
        } else {
            (*out_size) += read_len;
            if (*out_size >= head){
                *buffer = realloc(*buffer, head + 1024);
                head += 1024;
            }

            memcpy((*buffer) + ((*out_size) - read_len), buf, read_len);
            if (read_len < 1024)
                return 0;
        }
    }
    return 0;
}

int mfd_str_writeexact(mfd_stream str, size_t bytes, const void *buffer){
    if (!mfd_writeable(*str.tfd)){
        fprintf(stderr, "mfd_str_writeexact: fd is not writeable\n");
        return -2;
    }

    if (str.tfd->t == PROGRAM_FD){
        mfd_write(str.tfd, buffer, bytes);
        return 0;
    }
    
    size_t remain = bytes;
    while (remain > 0) {
        ssize_t write_len = mfd_write(str.tfd, buffer, remain);
        if (write_len < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                // EINTR: interrupted by system
                // EAGAIN: write is blocked
                // Try again
                continue;
            }
            return -1; // Unexpected error
        } else if (write_len == 0) {
            return -1; // No data from socket: disconnected.
        } else {
            // write success
            remain -= write_len;
            buffer += write_len;
        }
    }
    
    return 0;
}

void mfd_str_end(mfd_stream *str){
    if (!str) return;

    if (str->owner && str->tfd){
        free(str->tfd);
        str->owner = false;
        str->tfd = NULL;
    } else {
        str->owner = false;
        str->tfd = NULL;
    }
}

#endif
#define MFD_STREAM