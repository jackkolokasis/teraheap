
BUILDDIR = ./build
BINDIR   = $(BUILDDIR)/bin
INCDIR   = $(BUILDDIR)/include
LIBDIR   = $(BUILDDIR)/lib
OBJDIR   = $(BUILDDIR)/obj
LIBPATH  = $(LIBDIR)/libummapio.so
UMAPFLAG = -lummapio
CFLAGS   = -O2 -I. -L$(LIBDIR)
CFLAGS  += -Wall -Wno-unused-label -Wno-unused-function
CFLAGS  += -DDEBUG_PRINT=$(or $(DEBUG_PRINT),1)
LDFLAGS  = -pthread -lrt

all: setup $(LIBPATH) $(BINDIR)/test

setup:
	@mkdir -p $(BINDIR) $(INCDIR) $(LIBDIR) $(OBJDIR) 2> /dev/null
	@cp ./include/ummap.h $(INCDIR) 2> /dev/null

$(BINDIR)/test: $(LIBPATH) ./test/main.c
	$(CC) $(CFLAGS) ./test/main.c -o $(BINDIR)/test $(UMAPFLAG)

$(LIBPATH): $(OBJDIR)/ummap.o 
	@$(CC) $(OBJDIR)/*.o -shared -fPIC -o $(LIBPATH) $(LDFLAGS)

$(OBJDIR)/ummap.o: ./src/ummap.c ./include/ummap.h ./include/common.h
	@$(CC) $(CFLAGS) -c -fPIC ./src/ummap.c -o $(OBJDIR)/ummap.o

clean:
	@rm -rf $(BUILDDIR)

rebuild: clean all
