/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1997-2010 by Matthew Dillon, Dima Ruban, and Oliver Fromme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#include <signal.h>
#include <pwd.h>
#include <fnmatch.h>
#include <assert.h>

#ifdef __linux

/*
 * This is a horrible hack.  lchmod also seems to be missing
 * on the Debian system I am testing compatibility on (which will
 * break the symlink handling code), so not sure what to do about
 * that.
 *
 * XXX TODO
 */
#define lchmod	chmod	/* horrible hack */

#endif /* __linux */

#define VERSION	"1.22"
#define AUTHORS	"Matt Dillon, Dima Ruban, & Oliver Fromme"

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

#ifndef __aligned
#define __aligned(n) __attribute__((__aligned__(n)))
#endif

#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif

#ifndef __printflike
#define __printflike(a,b) \
	__attribute__((__nonnull__(a), __format__(__printf__, a, b)))
#endif

#ifndef __printf0like
#define __printf0like(a,b) __attribute__((__format__(__printf__, a, b)))
#endif

void logstd(const char *ctl, ...) __printflike(1, 2);
void logerr(const char *ctl, ...) __printflike(1, 2);
char *mprintf(const char *ctl, ...) __printflike(1, 2);
void fatal(const char *ctl, ...) __dead2 __printf0like(1, 2);
char *fextract(FILE *fi, int n, int *pc, int skip);

int16_t hc_bswap16(int16_t var);
int32_t hc_bswap32(int32_t var);
int64_t hc_bswap64(int64_t var);

#ifndef NOMD5
int md5_check(const char *spath, const char *dpath);
void md5_flush(void);
#endif

extern const char *UseCpFile;
extern const char *MD5CacheFile;
extern const char *UseHLPath;

extern int AskConfirmation;
extern int SafetyOpt;
extern int ForceOpt;
extern int DeviceOpt;
extern int VerboseOpt;
extern int DirShowOpt;
extern int QuietOpt;
extern int NotForRealOpt;
extern int NoRemoveOpt;
extern int UseMD5Opt;
extern int SlaveOpt;
extern int SummaryOpt;
extern int CompressOpt;
extern int ReadOnlyOpt;
extern int DstRootPrivs;
extern int ValidateOpt;

extern int ssh_argc;
extern const char *ssh_argv[];

extern int64_t CountSourceBytes;
extern int64_t CountSourceItems;
extern int64_t CountCopiedItems;
extern int64_t CountSourceReadBytes;
extern int64_t CountTargetReadBytes;
extern int64_t CountWriteBytes;
extern int64_t CountRemovedItems;
extern int64_t CountLinkedItems;

#ifdef DEBUG_MALLOC
void *debug_malloc(size_t bytes, const char *file, int line);
void debug_free(void *ptr);

#define malloc(bytes)	debug_malloc(bytes, __FILE__, __LINE__)
#define free(ptr)	debug_free(ptr)
#endif
