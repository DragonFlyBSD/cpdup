#
# CPDUP
#

CC?=	gcc
CFLAGS?=	-O -pipe -D_GNU_SOURCE -std=gnu99 -Wall
CFLAGS+=	$(shell pkg-config --cflags openssl)
LIBS?=	$(shell pkg-config --libs openssl)

PROG=	cpdup
SRCS=	cpdup.c hcproto.c hclink.c misc.c fsmid.c md5.c
ifeq ($(shell uname -s),Linux)
	SRCS+=	compat_linux.c compat_md5.c
endif
OBJS=	$(SRCS:.c=.o)

all: ${PROG}

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

clean:
	rm -f $(PROG) $(OBJS)

# Dependencies
cpdup.o: cpdup.c cpdup.h hclink.h hcproto.h
hcproto.o: hcproto.c cpdup.h hclink.h hcproto.h
hclink.o: hclink.c cpdup.h hclink.h hcproto.h
misc.o: misc.c cpdup.h
fsmid.o: fsmid.c cpdup.h
md5.o: md5.c cpdup.h

# One liner to get the value of any makefile variable
# Credit: http://blog.jgc.org/2015/04/the-one-line-you-should-add-to-every.html
print-%: ; @echo $*=$($*)
