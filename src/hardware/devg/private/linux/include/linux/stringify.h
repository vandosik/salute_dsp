#ifndef __LINUX_STRINGIFY_H
#define __LINUX_STRINGIFY_H

/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */

#ifndef __QNX4__
#define __stringify_1(x...)	#x
#define __stringify(x...)	__stringify_1(x)
#endif  /* __QNX4__ */

#endif	/* !__LINUX_STRINGIFY_H */
