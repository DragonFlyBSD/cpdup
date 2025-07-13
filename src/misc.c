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

#include "cpdup.h"

void
logstd(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vprintf(ctl, va);
    va_end(va);
}

void
logerr(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vfprintf(stderr, ctl, va);
    va_end(va);
}

char *
mprintf(const char *ctl, ...)
{
    char *ptr;
    va_list va;

    ptr = NULL;

    va_start(va, ctl);
    if (vasprintf(&ptr, ctl, va) < 0)
	fatal("malloc failed");
    va_end(va);
    assert(ptr != NULL);
    return(ptr);
}

int16_t
hc_bswap16(int16_t var)
{
    return ((var & 0xff) << 8 | (var >> 8 & 0xff));
}

int32_t
hc_bswap32(int32_t var)
{
    return ((var & 0xff) << 24 | (var & 0xff00) << 8
	    | (var >> 8 & 0xff00) | (var >> 24 & 0xff));
}

int64_t
hc_bswap64(int64_t var)
{
    return (hc_bswap32(var >> 32 & 0xffffffff)
	    | (int64_t) hc_bswap32(var & 0xffffffff) << 32);
}

#ifdef DEBUG_MALLOC

#undef malloc
#undef free

struct malloc_info {
	struct malloc_info *next;
	struct malloc_info *prev;
	const char *file;
	int magic;
	int line;
};

struct malloc_info DummyInfo = { &DummyInfo, &DummyInfo, NULL, 0, 0 };
struct malloc_info *InfoList = &DummyInfo;

void *
debug_malloc(size_t bytes, const char *file, int line)
{
	struct malloc_info *info = malloc(sizeof(*info) + bytes);

	info->magic = 0x5513A4C2;
	info->file = file;
	info->line = line;

	info->next = InfoList;
	info->prev = InfoList->prev;
	info->next->prev = info;
	info->prev->next = info;
	return(info + 1);
}

void
debug_free(void *ptr)
{
	struct malloc_info *info = (struct malloc_info *)ptr - 1;
	struct malloc_info *scan;
	static int report;

	for (scan = DummyInfo.next; scan != &DummyInfo; scan = scan->next) {
		if (info == scan) {
			assert(info->magic == 0x5513A4C2);
			info->magic = 0;
			info->next->prev = info->prev;
			info->prev->next = info->next;
			free(info);
			break;
		}
	}
	if (scan == &DummyInfo)
		free(ptr);

	if ((++report & 65535) == 0) {
		printf("--- report\n");
		for (scan = DummyInfo.next; scan != &DummyInfo; scan = scan->next) {
			printf("%-15s %d\n", scan->file, scan->line);
		}
	}
}

#endif /* DEBUG_MALLOC */

void
fatal(const char *ctl, ...)
{
    va_list va;

    if (ctl == NULL) {
	puts("usage: cpdup [options] src dest");
	puts("\n"
	     "options:\n"
	     "    -C          request compressed ssh link if remote operation\n"
	     "    -d          print directories being traversed\n"
	     "    -f          force update even if files look the same\n"
	     "    -F<ssh_opt> add <ssh_opt> to options passed to ssh\n"
	     "    -h          show this help\n"
	     "    -H path     hardlink from path to target instead of copying\n"
	     "    -I          display performance summary\n"
	     "    -i0         do NOT confirm when removing something\n"
	     "    -j0         do not try to recreate CHR or BLK devices\n"
	     "    -l          force line-buffered stdout/stderr"
	);
#ifndef NOMD5
	puts("    -m          maintain/generate MD5 checkfile on source,\n"
	     "                and compare with (optional) destination,\n"
	     "                copying if the compare fails\n"
	     "    -M file     -m+specify MD5 checkfile, else .MD5_CHECKSUMS\n"
	     "                copy if md5 check fails"
	);
#endif
	puts("    -n          do not make any real changes to the target\n"
	     "    -o          do not remove any files, just overwrite/add\n"
	     "    -q          quiet operation\n"
	     "    -R          read-only slave mode for ssh remotes\n"
	     "                source to target, if source matches path.\n"
	     "    -S          slave mode\n"
	     "    -s0         disable safeties - allow files to overwrite directories\n"
	     "    -u          use unbuffered output for -v[vv]\n"
	     "    -v[vv]      verbose level (-vv is typical)\n"
	     "    -V          verify file contents even if they appear\n"
	     "                to be the same.\n"
	     "    -VV         same as -V but ignore mtime entirely\n"
	     "    -x          use .cpignore as exclusion file\n"
	     "    -X file     specify exclusion file (can match full source\n"
	     "                path if the exclusion file is specified via\n"
	     "                an absolute path.\n"
	     "\n"
	     "Version " VERSION " by " AUTHORS "\n"
	);
	exit(0);
    } else {
	va_start(va, ctl);
	vfprintf(stderr, ctl, va);
	va_end(va);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
    }
}
