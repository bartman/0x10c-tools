TARGET = 0x10c
SRCS   = 0x10c.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

#LUA_PKGS   = lua5.1 lua5.1-lpeg
#LUA_LIBS   = $(shell pkg-config --libs ${LUA_PKGS})
#LUA_CFLAGS = $(shell pkg-config --cflags ${LUA_PKGS})

LIBS   = ../lib/lib0x10c.a

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

.PHONY: all clean

all: ${TARGET}

-include $(wildcard ${DEPS})
-include ../Makefile.conf

${TARGET}: ${OBJS} ${LIBS}
	${CC} ${LDFLAGS} ${CFLAGS} -o $@ $^ ${LUA_LIBS}

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${CPPFLAGS} -c -o $@ $<

${DEPS}: .%.dep: % Makefile
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${CPPFLAGS} -MM $< > $@

tags:
	${MAKE} -C .. $@

clean:
	-rm -f *~ *.o ${TARGET}

distclean: clean
	-rm -f .*.dep

