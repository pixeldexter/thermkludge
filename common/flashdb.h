#ifndef FLASHDB_H_INC
#define FLASHDB_H_INC

/* config */

/* if this is not defined, stream parameter is simply ignored */
// #define SUPPORT_MULTISTREAM 1


/* end config */

#include <stdint.h>
#include <stdlib.h>

/* copied from system errno.h */

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */



/*
 * flashdb definitions
 */

#define O_APPEND (1<<0)
#define O_READ   (1<<1)
#define O_WRITE  (1<<2)
#define O_SYNC   (1<<3)

typedef int8_t flashdb_return_type;



/*
 * flashdb api
 */

#ifdef SUPPORT_MULTISTREAM

flashdb_return_type flashdb_open(uint8_t stream, uint8_t flags);
flashdb_return_type flashdb_write(uint8_t stream, const void *buf, size_t sz);
flashdb_return_type flashdb_read(uint8_t stream, void *buf, size_t sz);
flashdb_return_type flashdb_close(uint8_t stream);

#else

flashdb_return_type flashdb_open(uint8_t flags);
flashdb_return_type flashdb_write(const void *buf, size_t sz);
flashdb_return_type flashdb_read(void *buf, size_t sz);
flashdb_return_type flashdb_close(void);

#endif

flashdb_return_type flashdb_erase(void);

#endif /* FLASHDB_H_INC */
