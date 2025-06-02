# Credit: http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
# NOTE: Include this makefile *after* the build target
#

DEPFLAGS=	-MT $@ -MMD -MP -MF $*.d
COMPILE.c=	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o: %.c
%.o: %.c %.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<

DEPFILES=	$(SRCS:%.c=%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))

.PHONY: cleandeps
clean: cleandeps
cleandeps:
	rm -f $(DEPFILES)
