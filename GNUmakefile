PROG=		cpdup
MAN=		cpdup.1
SRCS=		cpdup.c hcproto.c hclink.c misc.c fsmid.c md5.c
OBJS=		$(SRCS:.c=.o)

CFLAGS?=	-O -std=c99 -pedantic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=	-Wall -Wextra -Wlogical-op -Wshadow -Wformat=2 \
		-Wwrite-strings -Wcast-qual -Wcast-align
#CFLAGS+=	-Wduplicated-cond -Wduplicated-branches \
		-Wrestrict -Wnull-dereference \
#CFLAGS+=	-Wconversion

CFLAGS+=	$(shell pkg-config --cflags libbsd-overlay openssl)
LIBS?=		$(shell pkg-config --libs   libbsd-overlay openssl)

PREFIX?=	/usr/local

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install:
	install -s -Dm 0755 $(PROG) $(PREFIX)/bin/$(PROG)
	install -Dm 0644 $(MAN) $(PREFIX)/man/man1/$(MAN)
	gzip -9 $(PREFIX)/man/man1/$(MAN)

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all install clean

# Dependencies
cpdup.o: cpdup.c cpdup.h hclink.h hcproto.h
hcproto.o: hcproto.c cpdup.h hclink.h hcproto.h
hclink.o: hclink.c cpdup.h hclink.h hcproto.h
misc.o: misc.c cpdup.h
fsmid.o: fsmid.c cpdup.h
md5.o: md5.c cpdup.h
