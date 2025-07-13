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

#include <openssl/evp.h>

typedef struct MD5Node {
    struct MD5Node *md_Next;
    char *md_Name;
    char md_Code[EVP_MAX_MD_SIZE * 2 + 1]; /* hex-encoded digest */
    int md_Accessed;
} MD5Node;

static MD5Node *md5_lookup(const char *spath);
static void md5_cache(const char *spath, int sdirlen);
static void md5_load(FILE *fi);
static int md5_file(const char *filename, char *buf, int is_target);

static char *MD5SCache;		/* cache source directory name */
static MD5Node *MD5Base;
static int MD5SCacheDirLen;
static int MD5SCacheDirty;

void
md5_flush(void)
{
    MD5Node *node;
    FILE *fo;

    if (MD5SCacheDirty && MD5SCache && !NotForRealOpt) {
	if ((fo = fopen(MD5SCache, "w")) != NULL) {
	    for (node = MD5Base; node; node = node->md_Next) {
		if (node->md_Accessed && node->md_Code[0] != '\0') {
		    fprintf(fo, "%s %zu %s\n",
			node->md_Code,
			strlen(node->md_Name),
			node->md_Name
		    );
		}
	    }
	    fclose(fo);
	} else {
	    logerr("Error writing MD5 Cache (%s): %s\n",
		   MD5SCache, strerror(errno));
	}
    }

    MD5SCacheDirty = 0;

    if (MD5SCache) {
	while ((node = MD5Base) != NULL) {
	    MD5Base = node->md_Next;

	    if (node->md_Name != NULL)
		free(node->md_Name);
	    free(node);
	}
	free(MD5SCache);
	MD5SCache = NULL;
    }
}

static void
md5_cache(const char *spath, int sdirlen)
{
    FILE *fi;

    /*
     * Already cached
     */

    if (
	MD5SCache &&
	sdirlen == MD5SCacheDirLen &&
	strncmp(spath, MD5SCache, sdirlen) == 0
    ) {
	return;
    }

    /*
     * Different cache, flush old cache
     */
    if (MD5SCache != NULL)
	md5_flush();

    /*
     * Create new cache and load data if exists
     */
    MD5SCacheDirLen = sdirlen;
    MD5SCache = mprintf("%*.*s%s", sdirlen, sdirlen, spath, MD5CacheFile);
    if ((fi = fopen(MD5SCache, "r")) != NULL) {
	md5_load(fi);
	fclose(fi);
    } else if (errno != ENOENT) {
	logerr("Error reading MD5 Cache (%s): %s\n",
	       MD5SCache, strerror(errno));
    }
}

/*
 * md5_lookup:	lookup/create md5 entry
 */
static MD5Node *
md5_lookup(const char *spath)
{
    const char *sfile;
    int sdirlen;
    MD5Node *node;

    if ((sfile = strrchr(spath, '/')) != NULL)
	++sfile;
    else
	sfile = spath;
    sdirlen = sfile - spath;

    md5_cache(spath, sdirlen);

    for (node = MD5Base; node != NULL; node = node->md_Next) {
	if (strcmp(sfile, node->md_Name) == 0)
	    break;
    }
    if (node == NULL) {
	if ((node = malloc(sizeof(MD5Node))) == NULL)
	    fatal("out of memory");

	bzero(node, sizeof(MD5Node));
	node->md_Name = strdup(sfile);
	node->md_Next = MD5Base;
	MD5Base = node;
    }
    node->md_Accessed = 1;
    return(node);
}

/*
 * md5_update:	force update the source MD5 file.
 *
 *	Return -1 if failed
 *	Return 0  if up-to-date
 *	Return 1  if updated
 */
int
md5_update(const char *spath)
{
    char scode[EVP_MAX_MD_SIZE * 2 + 1];
    int r;
    MD5Node *node;

    node = md5_lookup(spath);

    if (md5_file(spath, scode, 0 /* is_target */) == 0) {
	r = 0;
	if (strcmp(scode, node->md_Code) != 0) {
	    r = 1;
	    bcopy(scode, node->md_Code, sizeof(scode));
	    MD5SCacheDirty = 1;
	}
    } else {
	r = -1;
    }

    return (r);
}

/*
 * md5_check:	check MD5 against file
 *
 *	Return -1 if check failed
 *	Return 0  if source and dest files are identical
 *	Return 1  if source and dest files are not identical
 */
int
md5_check(const char *spath, const char *dpath)
{
    char scode[EVP_MAX_MD_SIZE * 2 + 1];
    char dcode[EVP_MAX_MD_SIZE * 2 + 1];
    int r;
    MD5Node *node;

    node = md5_lookup(spath);

    /*
     * The .MD5* file is used as a cache.
     */
    if (md5_file(dpath, dcode, 1 /* is_target */) == 0) {
	r = 0;
	if (strcmp(node->md_Code, dcode) != 0) {
	    r = 1;
	    /*
	     * Update the source digest code and recheck.
	     */
	    if (md5_file(spath, scode, 0 /* is_target */) == 0) {
		if (strcmp(node->md_Code, scode) != 0) {
		    bcopy(scode, node->md_Code, sizeof(scode));
		    MD5SCacheDirty = 1;
		    if (strcmp(node->md_Code, dcode) == 0)
			r = 0;
		}
	    } else {
		r = -1;
	    }
	}
    } else {
	r = -1;
    }

    return(r);
}

/*
 * NOTE: buf will hold the hex-encoded digest and should have a size of
 *       >= (EVP_MAX_MD_SIZE * 2 + 1).
 */
static int
md5_file(const char *filename, char *buf, int is_target)
{
    static const char hex[] = "0123456789abcdef";
    unsigned char digest[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *ctx;
    unsigned char buffer[4096];
    struct stat st;
    off_t size;
    int fd, bytes;
    unsigned int i, md_len;

    ctx = NULL;
    fd = open(filename, O_RDONLY);
    if (fd < 0)
	goto err;
    if (fstat(fd, &st) < 0)
	goto err;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    ctx = EVP_MD_CTX_new();
#else
    ctx = EVP_MD_CTX_create();
#endif
    if (ctx == NULL)
	goto err;
    if (!EVP_DigestInit_ex(ctx, EVP_md5(), NULL))
	goto err;

    size = st.st_size;
    while (size > 0) {
	if ((size_t)size > sizeof(buffer))
	     bytes = read(fd, buffer, sizeof(buffer));
	else
	     bytes = read(fd, buffer, size);
	if (bytes < 0)
	     goto err;
	if (!EVP_DigestUpdate(ctx, buffer, bytes))
	     goto err;
	size -= bytes;
    }
    if (SummaryOpt) {
	if (is_target)
	    CountTargetReadBytes += st.st_size;
	else
	    CountSourceReadBytes += st.st_size;
    }

    if (!EVP_DigestFinal(ctx, digest, &md_len))
	goto err;

    close(fd);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    EVP_MD_CTX_free(ctx);
#else
    EVP_MD_CTX_destroy(ctx);
#endif

    for (i = 0; i < md_len; i++) {
	buf[2*i] = hex[digest[i] >> 4];
	buf[2*i+1] = hex[digest[i] & 0x0f];
    }
    buf[md_len * 2] = '\0';

    return (0);

err:
    if (fd >= 0)
	close(fd);
    if (ctx != NULL) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX_free(ctx);
#else
	EVP_MD_CTX_destroy(ctx);
#endif
    }
    return (-1);
}

static int
get_field(FILE *fi, int c, char *buf, size_t len)
{
    size_t n;

    n = 0;

    while (c != EOF) {
	if (c == ' ') {
	    buf[n] = '\0';
	    return (c);
	}

	buf[n++] = c;
	if (n == len)
	    break;

	c = fgetc(fi);
    }

    return (c);
}

static void
md5_load(FILE *fi)
{
    MD5Node **pnode = &MD5Base;
    MD5Node *node;
    int c, n, nlen;
    char nbuf[sizeof("2147483647")];
    char *endp;

    /*
     * Line format: "<code> <name_len> <name>"
     * - code: hex-encoded digest
     * - name_len: 10-based integer indicating the length of the file name
     * - name: the file name (may contain special characters)
     * Example: "359d5608935488c8d0af7eb2a350e2f8 7 cpdup.c"
     */
    c = fgetc(fi);
    while (c != EOF) {
	node = malloc(sizeof(MD5Node));
	if (node == NULL)
	    fatal("out of memory");
	memset(node, 0, sizeof(MD5Node));

	c = get_field(fi, c, node->md_Code, sizeof(node->md_Code));
	if (c != ' ') {
	    logerr("Error parsing MD5 Cache (%s): invalid digest code (%c)\n",
		   MD5SCache, c);
	    goto next;
	}

	c = fgetc(fi);
	c = get_field(fi, c, nbuf, sizeof(nbuf));
	if (c != ' ') {
	    logerr("Error parsing MD5 Cache (%s): invalid length (%c)\n",
		   MD5SCache, c);
	    goto next;
	}
	nlen = (int)strtol(nbuf, &endp, 10);
	if (*endp != '\0' || nlen == 0) {
	    logerr("Error parsing MD5 Cache (%s): invalid length (%s)\n",
		   MD5SCache, nbuf);
	    goto next;
	}

	if ((node->md_Name = malloc(nlen + 1)) == NULL)
	    fatal("out of memory");
	node->md_Name[nlen] = '\0';
	for (n = 0; n < nlen; n++) {
	    c = fgetc(fi);
	    if (c == EOF) {
		logerr("Error parsing MD5 Cache (%s): invalid filename\n",
		       MD5SCache);
		goto next;
	    }
	    node->md_Name[n] = c;
	}

	c = fgetc(fi);
	if (c != '\n' && c != EOF) {
	    logerr("Error parsing MD5 Cache (%s): trailing garbage (%c)\n",
		   MD5SCache, c);
	    while (c != EOF && c != '\n')
		c = fgetc(fi);
	}
	if (c == '\n')
	    c = fgetc(fi);

	node->md_Accessed = 1;
	*pnode = node;
	pnode = &node->md_Next;

	if (SummaryOpt) {
	    CountSourceReadBytes += strlen(node->md_Code) + strlen(nbuf) +
		nlen + 1;
	}
	continue;

    next:
	if (node->md_Name != NULL)
	    free(node->md_Name);
	free(node);
	while (c != EOF && c != '\n')
	    c = fgetc(fi);
	if (c == '\n')
	    c = fgetc(fi);
    }
}
