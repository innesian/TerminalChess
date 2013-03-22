CFLAGS=-Wall -g

chess: chess.o game.o
	gcc -std=c99 -o chess chess.o game.o

main.o: chess.c game.h
	gcc -std=c99 -c chess.c

game.o: game.c game.h
	gcc -std=c99 -c game.c

clean:
	rm -f *.o
