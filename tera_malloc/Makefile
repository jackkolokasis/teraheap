###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  08-04-2023 
# @email:    kolokasis@ics.forth.gr
#
###################################################

include ./Makefile_common.mk
include ./makefile.colors

#rules
.PHONY: all lib tera_create tera_malloc clean

all: tera_create tera_malloc

lib: $(TERALIB)

tera_create: $(TERA_CREATE_EXE)

tera_malloc: $(TERA_MALLOC_EXE)

$(TERA_CREATE_EXE): $(TERA_CREATE_OBJ) $(TERALIB)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(TERALIB) $(OFLAG) $@ 

$(TERA_MALLOC_EXE): $(TERA_MALLOC_OBJ) $(TERALIB)
	@echo -e '[$(BYELLOW)LNK$(RESET)]' $^
	@$(CC) $< $(TERALIB) $(OFLAG) $@ 

$(TERALIB): $(LIBOBJS)
	@echo -e '[$(BBLUE)MKDIR$(RESET)]' $(LIBDIR)
	@$(MKDIR) $(LIBDIR)
	@echo -e '[$(BYELLOW)LINK$(RESET)]' $(REGIONSLIBRARY)
	@$(CC) $(LIBOBJS) -shared -o $(TERALIB)

%.o: %.c
	@echo -e '[$(BGREEN)GCC$(RESET)]' $<
	@$(CC) $(CFLAGS) -fPIC $< $(OFLAG) $@

%.o: $(LIBHEADERS)

clean:
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBOBJ) $(TERA_CREATE_OBJ) $(TERA_MALLOC_OBJ)
	@$(RM) $(LIBOBJ) $(TERA_CREATE_OBJ) $(TERA_MALLOC_OBJ)

distclean: clean
	@echo -e '[$(BRED)RM$(RESET)]' $(TERA_CREATE_EXE) $(TERA_MALLOC_EXE)
	@$(RM) $(TERA_CREATE_EXE) $(TERA_MALLOC_EXE) 
	@echo -e '[$(BRED)RM$(RESET)]' $(TERALIB)
	@$(RM) $(TERALIB)
	@echo -e '[$(BRED)RM$(RESET)]' $(LIBDIR)
	@$(RM) $(LIBDIR)
