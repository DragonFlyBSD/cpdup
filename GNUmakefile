PROG=		cpdup
MAN=		cpdup.1
SRCS=		$(wildcard src/*.c)
OBJS=		$(SRCS:.c=.o)
DISTFILES=	GNUmakefile autodep.mk src $(MAN)
DISTFILES+=	BACKUPS PORTING LICENSE README.md

CFLAGS=		-g -O2 -pipe -std=c99 -pedantic
CFLAGS+=	-Wall -Wextra -Wlogical-op -Wshadow -Wformat=2 \
		-Wwrite-strings -Wcast-qual -Wcast-align
#CFLAGS+=	-Wduplicated-cond -Wduplicated-branches \
		-Wrestrict -Wnull-dereference \
#CFLAGS+=	-Wconversion

CFLAGS+=	$(shell pkg-config --cflags libcrypto)
LIBS+=		$(shell pkg-config --libs   libcrypto)

OS?=		$(shell uname -s)
ifeq ($(OS),FreeBSD)
CFLAGS+=	-D_ST_FLAGS_PRESENT_
else ifeq ($(OS),Linux)
CFLAGS+=	-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
endif

PREFIX?=	/usr/local
MAN_DIR?=	$(PREFIX)/share/man

TMPDIR?=	/tmp
RPMBUILD_DIR?=	$(TMPDIR)/$(PROG)-rpmbuild
ARCHBUILD_DIR?=	$(TMPDIR)/$(PROG)-archbuild
DEBBUILD_DIR?=	$(TMPDIR)/$(PROG)-debbuild
OUTPUT_DIR?=	$(CURDIR)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install:
	install -Dm 0755 $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)
	install -Dm 0644 $(MAN) $(DESTDIR)$(MAN_DIR)/man1/$(MAN)
	gzip -9 $(DESTDIR)$(MAN_DIR)/man1/$(MAN)

rpm:
	mkdir -p $(OUTPUT_DIR)
	rm -rf $(RPMBUILD_DIR)
	mkdir -p $(RPMBUILD_DIR)/BUILD
	cp -Rp $(DISTFILES) $(RPMBUILD_DIR)/BUILD/
	rpmbuild -bb -v \
		--define="_topdir $(RPMBUILD_DIR)" \
		linux/$(PROG).spec
	@arch=`uname -m` ; \
		pkg=`( cd $(RPMBUILD_DIR)/RPMS/$${arch}; ls $(PROG)-*.rpm )` ; \
		cp -v $(RPMBUILD_DIR)/RPMS/$${arch}/$${pkg} $(OUTPUT_DIR) ; \
		rm -rf $(RPMBUILD_DIR) ; \
		echo "Install with: 'sudo yum localinstall $${pkg}'"

archpkg:
	mkdir -p $(OUTPUT_DIR)
	rm -rf $(ARCHBUILD_DIR)
	mkdir -p $(ARCHBUILD_DIR)/src
	cp linux/PKGBUILD $(ARCHBUILD_DIR)/
	cp -Rp $(DISTFILES) $(ARCHBUILD_DIR)/src/
	( cd $(ARCHBUILD_DIR) && makepkg )
	@pkg=`( cd $(ARCHBUILD_DIR); ls $(PROG)-*.pkg.* )` ; \
		cp -v $(ARCHBUILD_DIR)/$${pkg} $(OUTPUT_DIR) ; \
		rm -rf $(ARCHBUILD_DIR) ; \
		echo "Install with: 'sudo pacman -U $${pkg}'"

debpkg:
	mkdir -p $(OUTPUT_DIR)
	rm -rf $(DEBBUILD_DIR)
	mkdir -p $(DEBBUILD_DIR)/build
	cp -Rp $(DISTFILES) $(DEBBUILD_DIR)/build/
	cp -Rp linux/debian $(DEBBUILD_DIR)/build/
	cd $(DEBBUILD_DIR)/build && \
		dpkg-buildpackage --build=binary --no-sign
	mv -v $(DEBBUILD_DIR)/$(PROG)*.deb $(OUTPUT_DIR)
	rm -rf $(DEBBUILD_DIR)
	@ cd $(OUTPUT_DIR) && printf "\nDebian packages:\n" && ls -1 *.deb

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all install clean rpm archpkg debpkg

include autodep.mk
