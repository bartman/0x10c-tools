TARGET = lib0x10c.a
SRCS   = 0x10c_isn.c 0x10c_parser.c 0x10c_generator.c 0x10c_exec.c 0x10c_vcpu.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

#LUA_PKGS   = lua5.1 lua5.1-lpeg
#LUA_CFLAGS = $(shell pkg-config --cflags ${LUA_PKGS})

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

.PHONY: all clean distclean

all: ${TARGET}

-include $(wildcard ${DEPS})
-include ../Makefile.conf

${TARGET}: ${OBJS}
	ar rcs $@ $^

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

