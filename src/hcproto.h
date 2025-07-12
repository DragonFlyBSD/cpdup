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

#ifndef _HCPROTO_H_
#define _HCPROTO_H_

#define HCPROTO_VERSION		6
#define HCPROTO_VERSION_COMPAT	2
#define HCPROTO_VERSION_LUCC	6	/* lutimes, lchflags, lchmod */

#define HC_HELLO	0x0001

#define HC_STAT		0x0010
#define HC_LSTAT	0x0011
#define HC_OPENDIR	0x0012
#define HC_READDIR	0x0013
#define HC_CLOSEDIR	0x0014
#define HC_OPEN		0x0015
#define HC_CLOSE	0x0016
#define HC_READ		0x0017
#define HC_WRITE	0x0018
#define HC_REMOVE	0x0019
#define HC_MKDIR	0x001A
#define HC_RMDIR	0x001B
#define HC_CHOWN	0x001C
#define HC_LCHOWN	0x001D
#define HC_CHMOD	0x001E
#define HC_LINK		0x001F
#define HC_CHFLAGS	0x0020
#define HC_READLINK	0x0021
#define HC_UMASK	0x0022
#define HC_SYMLINK	0x0023
#define HC_RENAME	0x0024
#define HC_UTIMES	0x0025
#define HC_MKNOD	0x0026
#define HC_GETEUID	0x0027
#define HC_GETGROUPS	0x0028
#define HC_SCANDIR	0x0029
#define HC_READFILE	0x002A
#define HC_LUTIMES	0x002B
#define HC_LCHFLAGS	0x002C
#define HC_LCHMOD	0x002D

#define LC_HELLOSTR	(0x0001|LCF_STRING)
#define LC_PATH1	(0x0010|LCF_STRING)
#define LC_PATH2	(0x0011|LCF_STRING)
#define LC_OFLAGS	(0x0012|LCF_INT32)
#define LC_MODE		(0x0013|LCF_INT32)
#define LC_BYTES	(0x0014|LCF_INT32)
#define LC_OWNER	(0x0015|LCF_INT32)
#define LC_GROUP	(0x0016|LCF_INT32)
#define LC_DEV		(0x0017|LCF_INT32)
#define LC_INO		(0x0018|LCF_INT64)
#define LC_NLINK	(0x0019|LCF_INT32)
#define LC_UID		(0x001A|LCF_INT32)
#define LC_GID		(0x001B|LCF_INT32)
#define LC_RDEV		(0x001C|LCF_INT32)
#define LC_ATIME	(0x001D|LCF_INT64)
#define LC_MTIME	(0x001E|LCF_INT64)
#define LC_CTIME	(0x001F|LCF_INT64)
#define LC_FILESIZE	(0x0020|LCF_INT64)
#define LC_FILEBLKS	(0x0021|LCF_INT64)
#define LC_FILEFLAGS	(0x0022|LCF_INT64)
#define LC_UNUSED23	(0x0023|LCF_INT64)
#define LC_DESCRIPTOR	(0x0024|LCF_INT32)
#define LC_DATA		(0x0025|LCF_BINARY)
#define LC_TYPE		(0x0026|LCF_INT32)
#define LC_BLKSIZE	(0x0027|LCF_INT32)
#define LC_VERSION	(0x0028|LCF_INT32)
#define LC_COUNT	(0x0029|LCF_INT32)
#define LC_ATIMENSEC	(0x002A|LCF_INT32)
#define LC_MTIMENSEC	(0x002B|LCF_INT32)
#define LC_CTIMENSEC	(0x002C|LCF_INT32)

#define XO_NATIVEMASK	3		/* passed through directly */
#define XO_CREAT	0x00010000
#define XO_EXCL		0x00020000
#define XO_TRUNC	0x00040000

#define HC_DESC_DIR	1
#define HC_DESC_FD	2

#ifndef NAME_MAX
#  ifdef MAXNAMLEN
#    define NAME_MAX	MAXNAMLEN
#  else
#    define NAME_MAX	255
#  endif
#endif

struct HCDirEntry {
	char d_name[NAME_MAX + 1];
};

int hc_connect(struct HostConf *hc, int readonly);
void hc_slave(int fdin, int fdout);

int hc_hello(struct HostConf *hc);
int hc_stat(struct HostConf *hc, const char *path, struct stat *st);
int hc_lstat(struct HostConf *hc, const char *path, struct stat *st);
DIR *hc_opendir(struct HostConf *hc, const char *path);
struct HCDirEntry *hc_readdir(struct HostConf *hc, DIR *dir, struct stat **statpp);
int hc_closedir(struct HostConf *hc, DIR *dir);
int hc_open(struct HostConf *hc, const char *path, int flags, mode_t mode);
int hc_close(struct HostConf *hc, int fd);
ssize_t hc_read(struct HostConf *hc, int fd, void *buf, size_t bytes);
ssize_t hc_write(struct HostConf *hc, int fd, const void *buf, size_t bytes);
int hc_remove(struct HostConf *hc, const char *path);
int hc_mkdir(struct HostConf *hc, const char *path, mode_t mode);
int hc_rmdir(struct HostConf *hc, const char *path);
int hc_chown(struct HostConf *hc, const char *path, uid_t owner, gid_t group);
int hc_lchown(struct HostConf *hc, const char *path, uid_t owner, gid_t group);
int hc_chmod(struct HostConf *hc, const char *path, mode_t mode);
int hc_lchmod(struct HostConf *hc, const char *path, mode_t mode);
int hc_mknod(struct HostConf *hc, const char *path, mode_t mode, dev_t rdev);
int hc_link(struct HostConf *hc, const char *name1, const char *name2);
int hc_chflags(struct HostConf *hc, const char *path, u_long flags);
int hc_lchflags(struct HostConf *hc, const char *path, u_long flags);
int hc_readlink(struct HostConf *hc, const char *path, char *buf, int bufsiz);
mode_t hc_umask(struct HostConf *hc, mode_t numask);
int hc_symlink(struct HostConf *hc, const char *name1, const char *name2);
int hc_rename(struct HostConf *hc, const char *name1, const char *name2);
int hc_utimes(struct HostConf *hc, const char *path, const struct timeval *times);
int hc_lutimes(struct HostConf *hc, const char *path, const struct timeval *times);
uid_t hc_geteuid(struct HostConf *hc);
int hc_getgroups(struct HostConf *hc, gid_t **gidlist);

#endif

