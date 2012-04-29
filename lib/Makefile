TARGET = libdcpu.a
SRCS   = dcpu_isn.c dcpu_parser.c dcpu_generator.c dcpu_exec.c dcpu_vcpu.c \
	 dcpu_clock.c dcpu_keyboard.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

#LUA_PKGS   = lua5.1 lua5.1-lpeg
#LUA_CFLAGS = $(shell pkg-config --cflags ${LUA_PKGS})

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

MKCONF = $(wildcard ../Makefile.conf)

.PHONY: all clean distclean

all: ${TARGET}

-include $(wildcard ${DEPS})
-include ${MKCONF}

${TARGET}: ${OBJS}
	ar rcs $@ $^

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${CPPFLAGS} -c -o $@ $<

${DEPS}: .%.dep: % Makefile ${MKCONF}
	${CC} ${CFLAGS} ${LUA_CFLAGS} ${CPPFLAGS} -MM $< > $@

tags:
	${MAKE} -C .. $@

clean:
	-rm -f *~ *.o ${TARGET}

distclean: clean
	-rm -f .*.dep

