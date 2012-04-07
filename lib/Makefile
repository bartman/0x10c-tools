TARGET = lib0x10c.a
SRCS   = 0x10c_isn.c
OBJS   = ${SRCS:.c=.o}
DEPS   = $(SRCS:%=.%.dep)

INCLUDES = ../include

CPPFLAGS = $(foreach p,${INCLUDES}, -I$p)

.PHONY: all clean

all: ${TARGET}

-include $(wildcard ${DEPS})

${TARGET}: ${OBJS}
	ar rcs $@ $^

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

${DEPS}: .%.dep: % Makefile
	${CC} ${CFLAGS} ${CPPFLAGS} -MM $< > $@

clean:
	-rm -f *~ *.o


