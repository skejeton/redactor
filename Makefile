all:
	cc ./src/buffer.c ./src/main.c -lSDL2 -lSDL2_ttf -fsanitize=address -g -lm

run: all
	./a.out

