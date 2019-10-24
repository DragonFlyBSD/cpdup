PROG=		cpdup
SRCS=		cpdup.c hcproto.c hclink.c misc.c fsmid.c md5.c
OBJS=		$(SRCS:.c=.o)

CFLAGS?=	-O -std=c99 -pedantic -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=	-Wall -Wextra \
		-Wduplicated-cond -Wduplicated-branches -Wlogical-op \
		-Wrestrict -Wnull-dereference -Wshadow -Wformat=2 \
		-Wwrite-strings -Wcast-qual -Wcast-align
#CFLAGS+=	-Wconversion

CFLAGS+=	$(shell pkg-config --cflags libbsd-overlay openssl)
LIBS?=		$(shell pkg-config --libs   libbsd-overlay openssl)

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
