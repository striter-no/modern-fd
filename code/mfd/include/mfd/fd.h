#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdbool.h>

#ifndef MFD_FD

typedef struct {
    int  rfd;
    bool writable;
    bool readable;
} r_fd;

r_fd mfd_rfd(
    int rfd, 
    bool writable, 
    bool readable
){
    return (r_fd){
        .readable = readable,
        .writable = writable,
        .rfd = rfd
    };
}

#endif
#define MFD_FD