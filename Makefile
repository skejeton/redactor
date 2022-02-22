all:
	cc ./src/buffer.c ./src/main.c -lSDL2 -lSDL2_ttf -lm -O3

run: all
	./a.out

