TARGET = lib0x10c.a
SRCS   = 0x10c_isn.c 0x10c_parser.c 0x10c_generator.c 0x10c_exec.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

.PHONY: all clean distclean

all: ${TARGET}

-include $(wildcard ${DEPS})
-include ../Makefile.conf

${TARGET}: ${OBJS}
	ar rcs $@ $^

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

${DEPS}: .%.dep: % Makefile
	${CC} ${CFLAGS} ${CPPFLAGS} -MM $< > $@

tags:
	${MAKE} -C .. $@

clean:
	-rm -f *~ *.o ${TARGET}

distclean: clean
	-rm -f .*.dep

