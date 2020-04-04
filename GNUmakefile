PROG=		cpdup
MAN=		cpdup.1
SRCS=		cpdup.c hcproto.c hclink.c misc.c fsmid.c md5.c
OBJS=		$(SRCS:.c=.o)

CFLAGS=		-O -pipe -std=c99 -pedantic
CFLAGS+=	-Wall -Wextra -Wlogical-op -Wshadow -Wformat=2 \
		-Wwrite-strings -Wcast-qual -Wcast-align
#CFLAGS+=	-Wduplicated-cond -Wduplicated-branches \
		-Wrestrict -Wnull-dereference \
#CFLAGS+=	-Wconversion

CFLAGS+=	$(shell pkg-config --cflags openssl)
LIBS+=		$(shell pkg-config --libs   openssl)

OS?=		$(shell uname -s)
ifeq ($(OS),FreeBSD)
CFLAGS+=	-D_ST_FLAGS_PRESENT_
else ifeq ($(OS),Linux)
CFLAGS+=	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=	$(shell pkg-config --cflags libbsd-overlay)
LIBS+=		$(shell pkg-config --libs   libbsd-overlay)
endif

PREFIX?=	/usr/local
MAN_DIR?=	$(PREFIX)/share/man

TMPDIR?=	/tmp
RPMBUILD_DIR?=	$(TMPDIR)/cpdup-rpmbuild
ARCHBUILD_DIR?=	$(TMPDIR)/cpdup-archbuild

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install:
	install -s -Dm 0755 $(PROG) $(PREFIX)/bin/$(PROG)
	install -Dm 0644 $(MAN) $(MAN_DIR)/man1/$(MAN)
	gzip -9 $(MAN_DIR)/man1/$(MAN)

rpm:
	mkdir -p $(RPMBUILD_DIR)
	rpmbuild -bb -v \
		--define="_sourcedir $(PWD)" \
		--define="_topdir $(RPMBUILD_DIR)" \
		linux/$(PROG).spec
	cp $(RPMBUILD_DIR)/RPMS/$(shell uname -m)/$(PROG)-*.rpm .
	rm -rf $(RPMBUILD_DIR)
	@echo "Install with: 'sudo rpm -ivh $(shell ls $(PROG)-*.rpm)'"

archpkg:
	mkdir -p $(ARCHBUILD_DIR)/src
	cp linux/PKGBUILD $(ARCHBUILD_DIR)/
	cp -pr * $(ARCHBUILD_DIR)/src
	( cd $(ARCHBUILD_DIR) && makepkg )
	cp $(ARCHBUILD_DIR)/$(PROG)-*.pkg.* .
	rm -rf $(ARCHBUILD_DIR)
	@echo "Install with: 'sudo pacman -U $(shell ls $(PROG)-*.pkg.*)'"

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all install clean rpm archpkg

# Dependencies
cpdup.o: cpdup.c cpdup.h hclink.h hcproto.h
hcproto.o: hcproto.c cpdup.h hclink.h hcproto.h
hclink.o: hclink.c cpdup.h hclink.h hcproto.h
misc.o: misc.c cpdup.h
fsmid.o: fsmid.c cpdup.h
md5.o: md5.c cpdup.h
