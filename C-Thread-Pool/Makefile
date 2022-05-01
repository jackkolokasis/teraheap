###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-03-2021 
# @email:    kolokasis@ics.forth.gr
#
###################################################

include ./Makefile_common.mk
include ./makefile.colors

#rules
.PHONY: clean

all: lib

lib: $(TPOOLLIBRARY)

$(TPOOLLIBRARY): $(LIBTPOOLOBJS)
	@echo -e '[$(BBLUE)MKDIR$(RESET)]' $(LIBDIR)
	@$(MKDIR) $(LIBDIR)
	@echo -e '[$(BYELLOW)LINK$(RESET)]' $(TPOOLLIBRARY)
	@$(CC) $(LIBTPOOLOBJS) -shared -o $(TPOOLLIBRARY) $(LDFLAGS)

%.o: %.c
	@echo -e '[$(BGREEN)GCC$(RESET)]' $<
	@$(CC) $(CFLAGS) -fPIC $< $(OFLAG) $@

%.o: $(LIBHEADERS)

clean:
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBTPOOLOBJS)
	@$(RM) $(LIBTPOOLOBJS)

distclean: clean
	@echo -e '[$(BRED)RM$(RESET)]' $(TPOOLLIBRARY)
	@$(RM) $(TPOOLLIBRARY)
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBDIR)
	@$(RM) $(LIBDIR)
