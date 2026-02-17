#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdio.h>
#include <unistd.h>
#include "fd.h"

#ifndef MFD_GFD

typedef struct {
    r_fd fd_r;
    r_fd fd_w;
} g_fd;

int mfd_gfd(g_fd *out, int flags){
    int fds[2];
    if (0 != pipe2(fds, flags)){
        perror("mfd_gfd:pipe2()");
        return -1;
    }

    out->fd_r = mfd_rfd(fds[0], false, true);
    out->fd_w = mfd_rfd(fds[1], true, false);
    
    return 0;
}

#endif
#define MFD_GFD