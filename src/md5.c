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
    char *md_Code;
    int md_Accessed;
} MD5Node;

static MD5Node *md5_lookup(const char *sfile);
static void md5_cache(const char *spath, int sdirlen);
static char *md5_file(const char *filename, char *buf, int is_target);

static char *MD5SCache;		/* cache source directory name */
static MD5Node *MD5Base;
static int MD5SCacheDirLen;
static int MD5SCacheDirty;

void
md5_flush(void)
{
    if (MD5SCacheDirty && MD5SCache && NotForRealOpt == 0) {
	FILE *fo;

	if ((fo = fopen(MD5SCache, "w")) != NULL) {
	    MD5Node *node;

	    for (node = MD5Base; node; node = node->md_Next) {
		if (node->md_Accessed && node->md_Code) {
		    fprintf(fo, "%s %zu %s\n",
			node->md_Code,
			strlen(node->md_Name),
			node->md_Name
		    );
		}
	    }
	    fclose(fo);
	}
    }

    MD5SCacheDirty = 0;

    if (MD5SCache) {
	MD5Node *node;

	while ((node = MD5Base) != NULL) {
	    MD5Base = node->md_Next;

	    if (node->md_Code)
		free(node->md_Code);
	    if (node->md_Name)
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
     * Create new cache
     */

    MD5SCacheDirLen = sdirlen;
    MD5SCache = mprintf("%*.*s%s", sdirlen, sdirlen, spath, MD5CacheFile);

    if ((fi = fopen(MD5SCache, "r")) != NULL) {
	MD5Node **pnode = &MD5Base;
	int c;

	c = fgetc(fi);
	while (c != EOF) {
	    MD5Node *node = *pnode = malloc(sizeof(MD5Node));
	    char *s;
	    int nlen;

	    nlen = 0;

	    if (pnode == NULL || node == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	    }

	    bzero(node, sizeof(MD5Node));
	    node->md_Code = fextract(fi, -1, &c, ' ');
	    node->md_Accessed = 1;
	    if ((s = fextract(fi, -1, &c, ' ')) != NULL) {
		nlen = strtol(s, NULL, 0);
		free(s);
	    }
	    /*
	     * extracting md_Name - name may contain embedded control
	     * characters.
	     */
	    CountSourceReadBytes += nlen+1;
	    node->md_Name = fextract(fi, nlen, &c, EOF);
	    if (c != '\n') {
		fprintf(stderr, "Error parsing MD5 Cache: %s (%c)\n", MD5SCache, c);
		while (c != EOF && c != '\n')
		    c = fgetc(fi);
	    }
	    if (c != EOF)
		c = fgetc(fi);
	    pnode = &node->md_Next;
	}
	fclose(fi);
    }
}

/*
 * md5_lookup:	lookup/create md5 entry
 */

static MD5Node *
md5_lookup(const char *sfile)
{
    MD5Node **pnode;
    MD5Node *node;

    for (pnode = &MD5Base; (node = *pnode) != NULL; pnode = &node->md_Next) {
	if (strcmp(sfile, node->md_Name) == 0) {
	    break;
	}
    }
    if (node == NULL) {

	if ((node = *pnode = malloc(sizeof(MD5Node))) == NULL) {
		fprintf(stderr,"out of memory\n");
		exit(EXIT_FAILURE);
	}

	bzero(node, sizeof(MD5Node));
	node->md_Name = strdup(sfile);
    }
    node->md_Accessed = 1;
    return(node);
}

/*
 * md5_check:  check MD5 against file
 *
 *	Return -1 if check failed
 *	Return 0  if check succeeded
 *
 * dpath can be NULL, in which case we are force-updating
 * the source MD5.
 */
int
md5_check(const char *spath, const char *dpath)
{
    const char *sfile;
    char *dcode;
    int sdirlen;
    int r;
    MD5Node *node;

    r = -1;

    if ((sfile = strrchr(spath, '/')) != NULL)
	++sfile;
    else
	sfile = spath;
    sdirlen = sfile - spath;

    md5_cache(spath, sdirlen);

    node = md5_lookup(sfile);

    /*
     * If dpath == NULL, we are force-updating the source .MD5* files
     */

    if (dpath == NULL) {
	char *scode = md5_file(spath, NULL, 0);

	r = 0;
	if (node->md_Code == NULL) {
	    r = -1;
	    node->md_Code = scode;
	    MD5SCacheDirty = 1;
	} else if (strcmp(scode, node->md_Code) != 0) {
	    r = -1;
	    free(node->md_Code);
	    node->md_Code = scode;
	    MD5SCacheDirty = 1;
	} else {
	    free(scode);
	}
	return(r);
    }

    /*
     * Otherwise the .MD5* file is used as a cache.
     */

    if (node->md_Code == NULL) {
	node->md_Code = md5_file(spath, NULL, 0);
	MD5SCacheDirty = 1;
    }

    dcode = md5_file(dpath, NULL, 1);
    if (dcode) {
	if (strcmp(node->md_Code, dcode) == 0) {
	    r = 0;
	} else {
	    char *scode = md5_file(spath, NULL, 0);

	    if (strcmp(node->md_Code, scode) == 0) {
		    free(scode);
	    } else {
		    free(node->md_Code);
		    node->md_Code = scode;
		    MD5SCacheDirty = 1;
		    if (strcmp(node->md_Code, dcode) == 0)
			r = 0;
	    }
	}
	free(dcode);
    }
    return(r);
}

static char *
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

    ctx = EVP_MD_CTX_new();
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

    if (!buf)
	buf = malloc(md_len * 2 + 1);
    if (!buf)
	goto err;

    close(fd);
    EVP_MD_CTX_free(ctx);

    for (i = 0; i < md_len; i++) {
	buf[2*i] = hex[digest[i] >> 4];
	buf[2*i+1] = hex[digest[i] & 0x0f];
    }
    buf[md_len * 2] = '\0';

    return buf;

err:
    if (fd >= 0)
	close(fd);
    if (ctx != NULL)
	EVP_MD_CTX_free(ctx);
    return NULL;
}
