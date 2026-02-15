#ifndef MFD_PFD_SIZE
#define MFD_PFD_SIZE 16384
#endif 

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gfd.h"

#ifndef MFD_PFD

typedef struct {
    uint8_t  buffer[MFD_PFD_SIZE];
    uint16_t used;
} p_fd;

p_fd mfd_pfd(){
    return (p_fd){
        {0},
        0
    };
}

int mfd_pfd_fbuf(char *buffer, size_t size, p_fd *out){
    if (size > MFD_PFD_SIZE) {
        fprintf(stderr, "mfd_pfd_fbuf: cannot create pfd with size of %zu (bigger than MFD_PFD_SIZE: %d)\n", size, MFD_PFD_SIZE);
        return -1;
    }

    memcpy(out->buffer, buffer, size);
    out->used = size;

    return 0;
}

#endif
#define MFD_PFD