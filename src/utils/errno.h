#ifndef UTILS_ERRNO_H_
#define UTILS_ERRNO_H_

#define EPERM         1 // Not permitted.
#define ENOENT        2 // Entry does not exist.
#define ESRCH         3 // No such process.
#define EINTR         4 // Interrupted system call.
#define EIO           5 // IO error.
#define ENXIO         6 // No such device or address.
#define E2BIG         7 // Argument list too big.
#define ENOEXEC       8 // Exec format error.
#define EBADF         9 // Bad file descriptor.
#define ECHILD       10 // No child process.
#define EAGAIN       11 // Try again.
#define ENOMEM       12 // Out of memory.
#define EACCES       13 // Permission denied.
#define EFAULT       14 // Bad access.
#define ENOTBLK      15 // Block device required.
#define EBUSY        16 // Device or resource busy.
#define EEXIST       17 // File exists.
#define EXDEV        18 // Cross link device.
#define ENODEV       19 // No such device.
#define ENOTDIR      20 // Not a directory.
#define EISDIR       21 // Is a directory.
#define EINVAL       22 // Invalid argument.
#define MAX_ERRNO  4095

#endif // UTILS_ERRNO_H_
