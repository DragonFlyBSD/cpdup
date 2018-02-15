/*
 * MD5-related compatible functions
 */

#include "cpdup.h"
#include "compat_md5.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

char *
MD5File(const char *filename, char *buf)
{
        unsigned char dbuf[8192];
        MD5_CTX ctx;
        int fd;
        int count;

        if ((fd = open(filename, O_RDONLY)) < 0)
                return (NULL);
        MD5_Init(&ctx);
        while ((count = read(fd, dbuf, sizeof(dbuf))) > 0)
                MD5_Update(&ctx, dbuf, count);
        close(fd);
        if (count < 0)
                return (NULL);
        if (buf == NULL)
                if ((buf = malloc(33)) == NULL)
                        return NULL;
        MD5_Final(dbuf, &ctx);
        for (count = 0; count < 16; count++)
                sprintf(buf + count * 2, "%02x", dbuf[count]);
        return (buf);
}
