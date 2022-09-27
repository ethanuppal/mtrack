PRG=main
CFLAGS+=-std=c99 -D MTRACK_AUTOLOG
WARNINGS=-Wall -Wextra

test: main.c tracker.c
	${CC} ${CFLAGS} ${WARNINGS} $^ -o ${PRG}

clean:
	rm -f main mtrack.log mtrace.analysis
