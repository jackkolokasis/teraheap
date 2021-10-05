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
.PHONY: all lib test1 test2 test3 test4 test5 test6 clean distclean test

all: test1 test2 test3 test4 test5 test6

lib: $(REGIONSLIBRARY)

test1: $(TEST1EXE)

test2: $(TEST2EXE)

test3: $(TEST3EXE)

test4: $(TEST4EXE)

test5: $(TEST5EXE)

test6: $(TEST6EXE)

$(TEST1EXE): $(TEST1OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TEST2EXE): $(TEST2OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@

$(TEST3EXE): $(TEST3OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@

$(TEST4EXE): $(TEST4OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@

$(TEST5EXE): $(TEST5OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@

$(TEST6EXE): $(TEST6OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@

$(REGIONSLIBRARY): $(LIBREGIONSOBJS)
	@echo -e '[$(BBLUE)MKDIR$(RESET)]' $(LIBDIR)
	@$(MKDIR) $(LIBDIR)
	@echo -e '[$(BYELLOW)LINK$(RESET)]' $(REGIONSLIBRARY)
	@$(CC) $(LIBREGIONSOBJS) -shared -o $(REGIONSLIBRARY) $(LDFLAGS)

%.o: %.c
	@echo -e '[$(BGREEN)GCC$(RESET)]' $<
	@$(CC) $(CFLAGS) -fPIC $< $(OFLAG) $@

%.o: $(LIBHEADERS)

test:
	./test3.bin

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
	@echo -e '[$(BRED)RM$(RESET)]' $(TEST1OBJ) $(TEST2OBJ) $(TEST3OBJ) $(TEST4OBJ) $(TEST5OBJ) $(TEST6OBJ)
	@$(RM) $(LIBREGIONSOBJS) $(TEST1OBJ) $(TEST2OBJ) $(TEST3OBJ) $(TEST4OBJ) $(TEST5OBJ) $(TEST6OBJ)

distclean: clean
	@echo -e '[$(BRED)RM$(RESET)]' $(TEST1EXE) $(TEST2EXE) $(TEST3EXE) $(TEST4EXE) $(TEST5EXE) $(TEST6EXE)
	@$(RM) $(TEST1EXE) $(TEST2EXE) $(TEST3EXE) $(TEST4EXE) $(TEST5EXE) $(TEST6EXE)
	@echo -e '[$(BRED)RM$(RESET)]' $(REGIONSLIBRARY)
	@$(RM) $(REGIONSLIBRARY)
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBDIR)
	@$(RM) $(LIBDIR)
