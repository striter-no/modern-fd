#ifndef MFD_PFD_SIZE
#define MFD_PFD_SIZE 16384
#endif 

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include "gfd.h"

#ifndef MFD_IMFD

typedef struct {
    uint8_t  buffer[MFD_PFD_SIZE];
    uint16_t used;
    int      pollin_ev;
    int      pollout_ev;
} im_fd;

im_fd mfd_imfd(){
    return (im_fd){
        {0},
        0,
        eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK),
        eventfd(1, EFD_CLOEXEC | EFD_NONBLOCK)
    };
}

void mfd_imfd_pollout(im_fd fd){
    uint64_t u = 1;
    write(fd.pollout_ev, &u, sizeof(u));
}

void mfd_imfd_pollin(im_fd fd){
    uint64_t u = 1;
    write(fd.pollin_ev, &u, sizeof(u));
}

void mfd_imfd_wpollout(im_fd fd){
    uint64_t u;
    read(fd.pollout_ev, &u, sizeof(u));
}

void mfd_imfd_wpollin(im_fd fd){
    uint64_t u;
    read(fd.pollin_ev, &u, sizeof(u));
}

int mfd_imfd_fbuf(char *buffer, size_t size, im_fd *out){
    if (size > MFD_PFD_SIZE) {
        fprintf(stderr, "mfd_imfd_fbuf: cannot create imfd with size of %zu (bigger than MFD_PFD_SIZE: %d)\n", size, MFD_PFD_SIZE);
        return -1;
    }

    memcpy(out->buffer, buffer, size);
    out->used = size;

    out->pollin_ev = eventfd(1, EFD_CLOEXEC | EFD_NONBLOCK);
    out->pollout_ev = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    return 0;
}

#endif
#define MFD_IMFD