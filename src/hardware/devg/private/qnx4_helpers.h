#ifdef __QNX__
#ifndef __QNXNTO__

#ifndef _QNX4_HELPERS_H
#define _QNX4_HELPERS_H


#include <errno.h>
#include <malloc.h>
#include <sys/sched.h>


#ifndef bool
typedef unsigned char           bool;
#define true                    (1)
#define false                   (0)
#endif

#define __signed__              signed
#define __always_inline

#endif  /* _QNX4_HELPERS_H */

#endif  /* __QNXNTO__ */
#endif  /* __QNX__ */
