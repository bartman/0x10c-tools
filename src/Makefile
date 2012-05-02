TARGET = dcpu-vm
SRCS   = dcpu_vm.c dcpu_vm_opts.c dcpu_vm_tracer.c dcpu_vm_curses.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

#LUA_PKGS   = lua5.1 lua5.1-lpeg
#LUA_LIBS   = $(shell pkg-config --libs ${LUA_PKGS})
#LUA_CFLAGS = $(shell pkg-config --cflags ${LUA_PKGS})

NC_PKGS    = ncurses
NC_LIBS    = $(shell pkg-config --libs ${NC_PKGS})
NC_CFLAGS  = $(shell pkg-config --cflags ${NC_PKGS})

LIBS   = ../lib/libdcpu.a

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

MKCONF = $(wildcard ../Makefile.conf)

.PHONY: all clean

all: ${TARGET}

-include $(wildcard ${DEPS})
-include ${MKCONF}

${TARGET}: ${OBJS} ${LIBS}
	${CC} ${LDFLAGS} ${CFLAGS} -o $@ $^ ${LUA_LIBS} ${NC_LIBS}

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${NC_CFLAGS} ${CPPFLAGS} -c -o $@ $<

${DEPS}: .%.dep: % Makefile ${MKCONF}
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${NC_CFLAGS} ${CPPFLAGS} -MM $< > $@

tags:
	${MAKE} -C .. $@

clean:
	-rm -f *~ *.o ${TARGET}

distclean: clean
	-rm -f .*.dep

