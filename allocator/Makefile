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
.PHONY: all clean distclean

all: test1 test2

lib: $(REGIONSLIBRARY)

test1: $(TEST1EXE)

test2: $(TEST2EXE)

$(TEST1EXE): $(TEST1OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TEST2EXE): $(TEST2OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(REGIONSLIBRARY): $(LIBREGIONSOBJS)
	@echo -e '[$(BBLUE)MKDIR$(RESET)]' $(LIBDIR)
	@$(MKDIR) $(LIBDIR)
	@echo -e '[$(BYELLOW)LINK$(RESET)]' $(REGIONSLIBRARY)
	@$(CC) $(LIBREGIONSOBJS) -shared -o $(REGIONSLIBRARY)

%.o: %.c
	@echo -e '[$(BGREEN)GCC$(RESET)]' $<
	@$(CC) $(CFLAGS) -fPIC $< $(OFLAG) $@

%.o: $(LIBHEADERS)


install:
	@echo -e '[$(BGREEN)INSTALL$(RESET)]' $(REGIONSLIBRARY)
	@$(CP) lib/libregions.so $(INSTALL_PREFIX)/lib/
	@echo -e '[$(BYELLOW)LN$(RESET)]' $(REGIONSLIBRARY)
	@ln -s $(INSTALL_PREFIX)/lib/libregions.so /usr/lib/libregions.so
	@echo -e '[$(BYELLOW)LN$(RESET)]' $(REGIONSLIBRARY)
	@ln -s $(INSTALL_PREFIX)/lib/libregions.so /usr/lib64/libregions.so
	@echo -e '[$(BGREEN)INSTALL$(RESET)]' $(INCLUDEDIR)/regions.h
	@$(CP) include/regions.h $(INSTALL_PREFIX)/include/

uninstall:
	@echo -e '[$(BRED)UNINSTALL$(RESET)]' /usr/lib/libregions.so
	@rm /usr/lib/libregions.so
	@echo -e '[$(BRED)UNINSTALL$(RESET)]' /usr/lib64/libregions.so
	@rm /usr/lib64/libregions.so
	@echo -e '[$(BRED)UNINSTALL$(RESET)]' $(INSTALL_PREFIX)/lib/libregions.so
	@rm $(INSTALL_PREFIX)/lib/libregions.so
	@echo -e '[$(BRED)UNINSTALL$(RESET)]' $(INSTALL_PREFIX)/include/regions.h
	@rm $(INSTALL_PREFIX)/include/regions.h

clean:
	@echo -e '[$(BRED)RM$(RESET)]' $(TEST1OBJ) $(TEST2OBJ)
	@$(RM) $(LIBREGIONSOBJS) $(TEST1OBJ) $(TEST2OBJ)

distclean: clean
	@echo -e '[$(BRED)RM$(RESET)]' $(TEST1EXE) $(TEST2EXE)
	@$(RM) $(TEST1EXE) $(TEST2EXE)
	@echo -e '[$(BRED)RM$(RESET)]' $(REGIONSLIBRARY)
	@$(RM) $(REGIONSLIBRARY)
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBDIR)
	@$(RM) $(LIBDIR)
