#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "tfd.h"

#ifndef MFD_METHODS

// RAW FD
ssize_t mfd_rwrite(r_fd fd, const void *buf, size_t n) {
    if (!fd.writable) { errno = EBADF; return -1; }
    return write(fd.rfd, buf, n);
}

ssize_t mfd_rread(r_fd fd, void *buf, size_t n_bytes) {
    if (!fd.readable) { errno = EBADF; return -1; }
    return read(fd.rfd, buf, n_bytes);
}

// GENERAL FD
ssize_t mfd_gwrite(g_fd fd, const void *buf, size_t n) {
    return mfd_rwrite(fd.fd_w, buf, n);
}

ssize_t mfd_gread(g_fd fd, void *buf, size_t n_bytes) {
    return mfd_rread(fd.fd_r, buf, n_bytes);
}

// PROGRAM FD
ssize_t mfd_pwrite(p_fd *fd, const void *buf, size_t n) {
    size_t available = MFD_PFD_SIZE - fd->used;
    if (available == 0) return 0;
    
    size_t to_write = (n < available) ? n : available;
    
    if (to_write > 0) {
        memcpy(fd->buffer + fd->used, buf, to_write);
        fd->used += to_write;
    }
    return (ssize_t)to_write;
}

ssize_t mfd_pread(p_fd *fd, void *buf, size_t n_bytes) {
    if (fd->used == 0) return 0;
    
    size_t to_read = (n_bytes < (size_t)fd->used) ? n_bytes : (size_t)fd->used;
    
    if (to_read > 0) {
        memcpy(buf, fd->buffer, to_read);
        memmove(fd->buffer, fd->buffer + to_read, fd->used - to_read);
        fd->used -= to_read;
    }
    return (ssize_t)to_read;
}

// IMMEDIATE FD

ssize_t mfd_iwrite(im_fd *fd, const void *buf, size_t n){
    mfd_imfd_wpollout(*fd);

    size_t available = MFD_PFD_SIZE - fd->used;
    if (available == 0) return 0;
    
    size_t to_write = (n < available) ? n : available;
    
    if (to_write > 0) {
        memcpy(fd->buffer + fd->used, buf, to_write);
        fd->used += to_write;
    }
    
    if (fd->used < MFD_PFD_SIZE){
        mfd_imfd_pollout(*fd);
    }

    mfd_imfd_pollin(*fd);

    return (ssize_t)to_write;
}

ssize_t mfd_iread(im_fd *fd, void *buf, size_t n_bytes) {
    mfd_imfd_wpollin(*fd);
    if (fd->used == 0) return 0;

    size_t to_read = (n_bytes < (size_t)fd->used) ? n_bytes : (size_t)fd->used;
    
    if (to_read > 0) {
        memcpy(buf, fd->buffer, to_read);
        memmove(fd->buffer, fd->buffer + to_read, fd->used - to_read);
        fd->used -= to_read;
    }

    if (fd->used != 0){
        mfd_imfd_pollin(*fd);
    }

    mfd_imfd_pollout(*fd);
    return (ssize_t)to_read;
}

// TAGGED FD
ssize_t mfd_twrite(t_fd *fd, const void *buf, size_t n) {
    switch (fd->t) {
        case RAW_FD:     return mfd_rwrite(fd->fd.rfd, buf, n);
        case GENERAL_FD: return mfd_gwrite(fd->fd.gfd, buf, n);
        case PROGRAM_FD: return mfd_pwrite(fd->fd.pfd, buf, n);
        case IMM_FD:     return mfd_iwrite(fd->fd.ifd, buf, n);
        default:         return -1;
    }
}

ssize_t mfd_tread(t_fd *fd, void *buf, size_t n_bytes) {
    switch (fd->t) {
        case RAW_FD:     return mfd_rread(fd->fd.rfd, buf, n_bytes);
        case GENERAL_FD: return mfd_gread(fd->fd.gfd, buf, n_bytes);
        case PROGRAM_FD: return mfd_pread(fd->fd.pfd, buf, n_bytes);
        case IMM_FD:     return mfd_iread(fd->fd.ifd, buf, n_bytes);
        default:         return -1;
    }
}

// Generic methods
#define mfd_write(fd, buf, n) _Generic((fd), \
    r_fd:  mfd_rwrite,                       \
    g_fd:  mfd_gwrite,                       \
    p_fd*: mfd_pwrite,                       \
    im_fd*: mfd_iwrite,                      \
    t_fd*: mfd_twrite                        \
)(fd, buf, n)

#define mfd_read(fd, buf, n) _Generic((fd), \
    r_fd:  mfd_rread,                      \
    g_fd:  mfd_gread,                      \
    p_fd*: mfd_pread,                      \
    im_fd*: mfd_iread,                     \
    t_fd*: mfd_tread                       \
)(fd, buf, n)

// Additional useful methods

// Read from src, write to dest
ssize_t mfd_redirect(t_fd *dest, t_fd *src) {
    char buff[BUFSIZ];
    ssize_t jread = mfd_read(src, buff, BUFSIZ);
    
    if (jread <= 0) {
        return jread;
    }

    ssize_t total_written = 0;
    while (total_written < jread) {
        ssize_t jwrite = mfd_write(dest, buff + total_written, jread - total_written);
        
        if (jwrite == -1) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            return -1; 
        }
        
        if (jwrite == 0 && (jread - total_written) > 0) {
            break; 
        }
        
        total_written += jwrite;
    }

    return total_written;
}

// Get available N of bytes for reading
size_t mfd_ravailable(r_fd fd){
    int nbytes = 0;
    if (ioctl(fd.rfd, FIONREAD, &nbytes) < 0) {
        return 0;
    }
    return (size_t)nbytes;
}

// Get available N of bytes for reading
size_t mfd_gavailable(g_fd fd){
    int nbytes = 0;
    if (ioctl(fd.fd_r.rfd, FIONREAD, &nbytes) < 0) {
        return 0;
    }
    return (size_t)nbytes;
}

// Get available N of bytes for reading
size_t mfd_pavailable(p_fd fd){
    return fd.used;
}

// Get available N of bytes for reading
size_t mfd_available(t_fd fd) {
    switch (fd.t) {
        case RAW_FD:     return mfd_ravailable(fd.fd.rfd);
        case GENERAL_FD: return mfd_gavailable(fd.fd.gfd);
        case PROGRAM_FD: return mfd_pavailable(*fd.fd.pfd);
        default: return 0;
    }
}

int mfd_set_flags(r_fd fd, int flags){
    return fcntl(fd.rfd, F_SETFL, flags);
}

int mfd_get_flags(r_fd fd){
    return fcntl(fd.rfd, F_GETFL, 0);
}

#endif
#define MFD_METHODS