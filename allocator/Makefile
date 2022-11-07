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
.PHONY: all lib tc_allocate tc_group tc_free tc_sync tc_async clean distclean test depedencies

all: tc_allocate tc_group tc_free tc_sync tc_async tc_multi_region

lib: $(REGIONSLIBRARY)

tc_allocate: $(TC_ALLOCATE_EXE)

tc_group: $(TC_GROUP_EXE)

tc_free: $(TC_FREE_EXE)

tc_sync: $(TC_SYNC_EXE)

tc_async: $(TC_ASYNC_EXE)

tc_multi_region: $(TC_ALLOCATE_MULTI_REGION_EXE)

$(TC_ALLOCATE_MULTI_REGION_EXE): $(TC_ALLOCATE_MULTI_REGION) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TC_ALLOCATE_EXE): $(TC_ALLOCATE_OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TC_GROUP_EXE): $(TC_GROUP_OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TC_FREE_EXE): $(TC_FREE_OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TC_SYNC_EXE): $(TC_SYNC_OBJ) $(REGIONSLIBRARY)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(REGIONSLIBRARY) $(OFLAG) $@ 

$(TC_ASYNC_EXE): $(TC_ASYNC_OBJ) $(REGIONSLIBRARY)
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
	@echo -e '[$(BRED)RM$(RESET)]' $(TC_ALLOCATE_OBJ) $(TC_SYNC_OBJ) $(TC_ASYNC_OBJ) $(TC_GROUP_OBJ) $(TC_FREE_OBJ)
	@$(RM) $(LIBREGIONSOBJS) $(TC_ALLOCATE_OBJ) $(TC_SYNC_OBJ) $(TC_ASYNC_OBJ) $(TC_GROUP_OBJ) $(TC_FREE_OBJ) $(TC_ALLOCATE_MULTI_REGION)

distclean: clean
	@echo -e '[$(BRED)RM$(RESET)]' $(TC_ALLOCATE_EXE) $(TC_SYNC_EXE) $(TC_ASYNC_EXE) $(TC_GROUP_EXE) $(TC_FREE_EXE)
	@$(RM) $(TC_ALLOCATE_EXE) $(TC_SYNC_EXE) $(TC_ASYNC_EXE) $(TC_GROUP_EXE) $(TC_FREE_EXE) $(TC_ALLOCATE_MULTI_REGION_EXE)
	@echo -e '[$(BRED)RM$(RESET)]' $(REGIONSLIBRARY)
	@$(RM) $(REGIONSLIBRARY)
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBDIR)
	@$(RM) $(LIBDIR)

test:
	@for t in $$(ls *.bin); do \
		./$$t 2> out; \
	done 
	@rm out

dependencies:
	@test ! -d "C-Thread-Pool" && git clone git@github.com:Pithikos/C-Thread-Pool.git
