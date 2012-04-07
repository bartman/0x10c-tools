TARGET = 0x10c
SRCS   = 0x10c.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

LIBS   = ../lib/lib0x10c.a

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

.PHONY: all clean

all: ${TARGET}

-include $(wildcard ${DEPS})

${TARGET}: ${OBJS} ${LIBS}
	${CC} ${LDFLAGS} ${CFLAGS} -o $@ $^

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

