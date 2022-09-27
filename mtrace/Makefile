PRG=mtrace
CFLAGS+=-std=c99
WARNINGS=-Wall -Wextra

SRC=$(wildcard *.c)
OBJ=${SRC:.c=.o}

${PRG}: ${OBJ}
	${CC} ${CFLAGS} ${WARNINGS} -O2 -g -O0 $^ -o ${PRG}

clean:
	rm -f ${PRG} ${OBJ}
