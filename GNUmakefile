#
# cpdup makefile for GNU/Linux
#
# (requires openssl and libssl-dev)
#

PROG=		cpdup
OS=		$(shell uname -s)
SRCS=		cpdup.c hcproto.c hclink.c misc.c fsmid.c md5.c
ifeq ($(OS),Linux)
SRCS+=		compat_linux.c compat_md5.c
endif
OBJS=		$(SRCS:.c=.o)

CFLAGS?=	-O -pipe -D_GNU_SOURCE -std=gnu99 -Wall -Wextra
CFLAGS+=	$(shell pkg-config --cflags openssl)
LIBS?=		$(shell pkg-config --libs openssl)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)

# Dependencies
cpdup.o: cpdup.c cpdup.h hclink.h hcproto.h
hcproto.o: hcproto.c cpdup.h hclink.h hcproto.h
hclink.o: hclink.c cpdup.h hclink.h hcproto.h
misc.o: misc.c cpdup.h
fsmid.o: fsmid.c cpdup.h
md5.o: md5.c cpdup.h


print-%: ;  @echo $*=$($*)
