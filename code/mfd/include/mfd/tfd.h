#include "pfd.h"

#ifndef MFD_TFD

typedef enum {
    GENERAL_FD,
    RAW_FD,
    PROGRAM_FD
} t_fd_type;

typedef struct {
    union {
        g_fd gfd;
        r_fd rfd;
        p_fd *pfd;
    } fd;

    t_fd_type t;
} t_fd;

int mfd_tfd(void *generic_fd, t_fd_type t, t_fd *out){
    out->t = t;
    switch (t) {
        case GENERAL_FD: 
            out->fd.gfd = *(const g_fd*)generic_fd;
        return 0;
        case RAW_FD: 
            out->fd.rfd = *(const r_fd*)generic_fd;
        return 0;
        case PROGRAM_FD: 
            out->fd.pfd = (p_fd*)generic_fd;
        return 0;
        default:
            return -1;
    }
}

// Do not use on pfd
int tfd_get(t_fd fd, int readm){
    switch (fd.t) {
        case GENERAL_FD: 
            return readm == 0 ? fd.fd.gfd.fd_r.rfd: fd.fd.gfd.fd_w.rfd;
        case RAW_FD: 
            return fd.fd.rfd.rfd;
        case PROGRAM_FD: 
            return -1;
        default:
            return -1;
    }
}

bool mfd_readable(t_fd fd){
    switch (fd.t) {
        case GENERAL_FD: 
            return fd.fd.gfd.fd_r.readable;
        case RAW_FD: 
            return fd.fd.rfd.readable;
        case PROGRAM_FD: 
            return true;
        default:
            return false;
    }
}

bool mfd_writeable(t_fd fd){
    switch (fd.t) {
        case GENERAL_FD: 
            return fd.fd.gfd.fd_w.writable;
        case RAW_FD: 
            return fd.fd.rfd.writable;
        case PROGRAM_FD: 
            return true;
        default:
            return false;
    }
}

#endif
#define MFD_TFD