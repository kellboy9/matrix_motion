CC=gcc
CFLAGS=-lm -std=c99 -lSDL2 -llua -Wall -Werror -pedantic -Wno-parentheses -g
matrix_motion: matrix_motion.o map.o calc.o entity.o list.o dialogue.o
	$(CC) $(CFLAGS) -o matrix_motion matrix_motion.o map.o calc.o entity.o list.o dialogue.o
matrix_motion.o: matrix_motion.c
	$(CC) $(CFLAGS) -o matrix_motion.o -c matrix_motion.c
map: map.c
	$(CC) $(CFLAGS) -o map.o -c map.c
calc.o: calc.c
	$(CC) $(CFLAGS) -o calc.o -c calc.c
entity.o: entity.c
	$(CC) $(CFLAGS) -o entity.o -c entity.c
list.o: list.c
	$(CC) $(CFLAGS) -o list.o -c list.c
dialogue.o: dialogue.c
	$(CC) $(CFLAGS) -o dialogue.o -c dialogue.c

clean:
	rm matrix_motion *.o
