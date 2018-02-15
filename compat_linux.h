/*
 * Implement some functions which are unknown to Linux.
 */

#ifndef COMPAT_LINUX_H_
#define COMPAT_LINUX_H_

#include <sys/types.h>

size_t strlcpy(char *, const char *, size_t);

#endif  /* COMPAT_LINUX_H_ */
