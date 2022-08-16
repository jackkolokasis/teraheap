
BUILDDIR = ../build
BINDIR   = $(BUILDDIR)/bin
INCDIR   = $(BUILDDIR)/include
LIBDIR   = $(BUILDDIR)/lib
OBJDIR   = $(BUILDDIR)/obj
LIBPATH  = $(LIBDIR)/libummapio.so
CLIBS    = -lummapio
CFLAGS   = -DDEBUG_PRINT=$(or $(DEBUG_PRINT),0) -O2 -I. -L$(LIBDIR)
LDFLAGS  = -pthread -lrt

ifeq (,$(findstring Cray,$(shell cc -V 2>&1)))
    CLIBS  += -pthread -lrt
    CFLAGS += -Wall -Wno-unused-label -Wno-unused-function -std=c99 -D_XOPEN_SOURCE=500
endif

all: setup $(LIBPATH) $(BINDIR)/example.out

$(BINDIR)/example.out: $(LIBPATH) example/main.c
	$(CC) $(CFLAGS) example/main.c -o $(BINDIR)/example.out $(CLIBS)

$(LIBPATH): $(OBJDIR)/ummap.o $(OBJDIR)/ummap_util.o \
			$(OBJDIR)/ummap_policy.o $(OBJDIR)/futex.o
	@$(CC) $(OBJDIR)/*.o -shared -fPIC -o $(LIBPATH) $(LDFLAGS)

$(OBJDIR)/ummap.o: ummap.* common.h
	$(CC) $(CFLAGS) -c -fPIC ummap.c -o $(OBJDIR)/ummap.o

$(OBJDIR)/ummap_util.o: ummap_util.* common.h
	$(CC) $(CFLAGS) -c -fPIC ummap_util.c -o $(OBJDIR)/ummap_util.o

$(OBJDIR)/ummap_policy.o: ummap_policy.* common.h
	$(CC) $(CFLAGS) -c -fPIC ummap_policy.c -o $(OBJDIR)/ummap_policy.o

$(OBJDIR)/futex.o: futex.* common.h
	$(CC) $(CFLAGS) -c -fPIC futex.c -o $(OBJDIR)/futex.o

setup:
	@mkdir -p $(BINDIR) $(INCDIR) $(LIBDIR) $(OBJDIR) 2> /dev/null
	@cp ummap.h $(INCDIR) 2> /dev/null

clean:
	@rm -rf $(BUILDDIR)

rebuild: clean all

